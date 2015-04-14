#include "gpk.h"
#include "stream.h"
#include <zlib.h>
#include <string.h>
#include <wchar.h>

#include <sys/stat.h>
#include <errno.h>

GPK::GPK()
{
    this->m_entries_count = 0;
}

GPK::~GPK()
{
#ifdef SD_DEBUG
    printf("GPK: free\n");
#endif
    uint32_t size = this->m_entries.size();
    for (uint32_t i=0 ; i<size ; i++)
    {
        delete this->m_entries.at(i)->name;
        delete this->m_entries.at(i);
    }
    this->m_entries.clear();
}

int u8_toutf8(char *dest, int sz, uint16_t *src, int srcsz)
{
    uint16_t ch;
    int i = 0;
    char *dest_end = dest + sz;

    while (srcsz<0 ? src[i]!=0 : i < srcsz)
    {
        ch = src[i];
        if (ch < 0x80)
        {
            if (dest >= dest_end)
                return i;
            *dest++ = (char)ch;
        }
        else if (ch < 0x800)
        {
            if (dest >= dest_end-1)
                return i;
            *dest++ = (ch>>6) | 0xC0;
            *dest++ = (ch & 0x3F) | 0x80;
        }
        i++;
    }
    if (dest < dest_end)
        *dest = '\0';
    return i;
}

bool GPK::load(const char* file_name)
{
    strcpy(this->m_file_name, file_name);

    Stream *str = new Stream(file_name, FILE_OPEN_READ_ST);
    if (str->getFileStreamHandle() == INVALID_HANDLE_VALUE)
    {
        ERROR_MESSAGE("GPK: %s fail to open\n", file_name);
        return false;
    }

    GPK_sig_t GPK_sig;
    str->seek(sizeof(GPK_sig_t) * (-1LL), spEnd);
    //str->seek(str->getSize() - sizeof(GPK_sig_t), spEnd);
    str->getPosition();
    if ((str->read(&GPK_sig, sizeof(GPK_sig_t)) != sizeof(GPK_sig_t))
            || strncmp(GPK_TAILER_IDENT0, GPK_sig.sig0, strlen(GPK_TAILER_IDENT0))
            || strncmp(GPK_TAILER_IDENT1, GPK_sig.sig1, strlen(GPK_TAILER_IDENT1)))
    {
        ERROR_MESSAGE("GPK: broken sign\n");
        delete str;
        return false;
    }

    char* comp = new char[GPK_sig.pidx_length];
    str->seek(str->getSize() - (sizeof(GPK_sig_t) + GPK_sig.pidx_length), spBegin);
    if (str->read(comp, GPK_sig.pidx_length) != GPK_sig.pidx_length)
    {
        delete str;
        return false;
    }
    delete str;

    this->decode(comp, GPK_sig.pidx_length);
    uint32_t uncomprlen = *(uint32_t*)comp;
    if (!uncomprlen)
    {
        ERROR_MESSAGE("GPK: compressed data zero length\n");
        delete comp;
        return false;
    }

    char* uncompr = new char[uncomprlen];
    unsigned long act_uncomprlen = uncomprlen;
    if (uncompress((Bytef*)uncompr, &act_uncomprlen, (Bytef*)(comp + 4),
                   GPK_sig.pidx_length - 4) != Z_OK)
    {
        ERROR_MESSAGE("GPK: uncompress failed\n");
        delete uncompr;
        delete comp;
        return false;
    }

    char *unc = uncompr,
          *unc_end = uncompr + uncomprlen;
    while (unc < unc_end)
    {
        GPK_entry_t *entry = new GPK_entry_t;

        uint16_t fn_l = *(uint16_t*)unc;
        unc += sizeof(uint16_t);

        entry->name = NULL;
        entry->name = new char[fn_l + 1];
        u8_toutf8(entry->name, MAX_PATH, (uint16_t*)unc, fn_l);
        unc += fn_l*2;

        entry->header = *(GPK_pidx_t*)unc;
        unc += sizeof(GPK_pidx_t);

        this->m_entries.push_back(entry);
    }

    delete []uncompr;
    delete []comp;
    return true;
}

void GPK::decode(char* buf, int size)
{
    int k = 0;
    for (int i=0 ; i<size ; i++)
    {
        buf[i] ^= CIPHERCODE[k++];
        if (k >= 16)
            k = 0;
    }
}

const char *GPK::get_file_name()
{
    return this->m_file_name;
}

GPK_pidx_t *GPK::get_entry(const char *file_name)
{
    uint32_t size = this->m_entries.size();
    for (uint32_t i=0 ; i<size ; i++)
    {
        if (strcasecmp(this->m_entries.at(i)->name, file_name) == 0)
        {
            return &this->m_entries.at(i)->header;
        }
    }
    return NULL;
}

#define BUF_SIZE 1024*100

typedef struct stat Stat;

static int do_mkdir(const char *path, mode_t mode)
{
    Stat            st;
    int             status = 0;

    if (stat(path, &st) != 0)
    {
        /* Directory does not exist. EEXIST for race condition */
#ifdef _WIN32
        if (mkdir(path) != 0 && errno != EEXIST)
#else
        if (mkdir(path, mode) != 0 && errno != EEXIST)
#endif
            status = -1;
    }
    else if (!S_ISDIR(st.st_mode))
    {
        errno = ENOTDIR;
        status = -1;
    }

    return(status);
}

char *STRDUP(const char *str)
{
    char *res = new char[strlen(str)];
    strcpy(res, str);
    return res;
}

/**
** mkpath - ensure all directories in path exist
** Algorithm takes the pessimistic view and works top-down to ensure
** each directory in path exists, rather than optimistically creating
** the last element and working backwards.
*/
int mkpath(const char *path, mode_t mode)
{
    char           *pp;
    char           *sp;
    int             status;
    char           *copypath = STRDUP(path);

    status = 0;
    pp = copypath;
    while (status == 0 && (sp = strchr(pp, '/')) != 0)
    {
        if (sp != pp)
        {
            /* Neither root nor double slash in path */
            *sp = '\0';
            status = do_mkdir(copypath, mode);
            *sp = '/';
        }
        pp = sp + 1;
    }
    if (status == 0)
        status = do_mkdir(path, mode);
    delete copypath;
    return (status);
}

void GPK::extract(const char *dir)
{
    //for (uint32_t i=0 ; i<this->m_entries.size() ; i++)
    int i = 0;
    uint32_t size = this->m_entries.size();
    GPK_entries_t::iterator end = this->m_entries.end();
    for (GPK_entries_t::iterator it=this->m_entries.begin() ; it<end ; it++)
    {
        i++;
        GPK_entry_t *item = *it;
        if (strlen(item->name) == 0)
            break;

        NORMA_MESSAGE("GPK: Extract (%i/%ui) %s \n", i, size, item->name);
        string fn = string(dir) + item->name;

        mkpath(fn.substr(0, fn.find_last_of("/")).c_str(), 0777);

        GPK_pidx_t* pidx = &item->header;
        Stream *src = new Stream(this);
        if (pidx->uncomprlen)
            src->m_data.size = pidx->uncomprlen;
        else
            src->m_data.size = pidx->comprlen;
        src->m_data.offset = pidx->offset;
        src->m_data.stream = new Stream(this->m_file_name, FILE_OPEN_READ_ST);

        Stream *dst = new Stream(fn.c_str(), FILE_OPEN_WRITE_ST);
        char buf[BUF_SIZE];
        while (src->getPosition() < src->getSize())
        {
            StreamSize readed = src->read(buf, BUF_SIZE);
            dst->write(buf, readed);
        }
        delete dst;
        delete src;
    }
}

StreamSize GPK::seek(Stream* stream, StreamSize moveTo, ESeekMethod method)
{
    uint32_t newPos;
    switch (method)
    {
    case spBegin:
        newPos = moveTo;
        break;
    case spCurrent:
        newPos = stream->m_data.position + moveTo;
        break;
    case spEnd:
        newPos = stream->m_data.size + moveTo;
        break;
    default:
        newPos = stream->m_data.position;
    }
    if (newPos > stream->m_data.size)
        stream->setSize(newPos);
    stream->m_data.position = newPos;
    return newPos;
}

StreamSize GPK::getSize(Stream* stream)
{
    return stream->m_data.size;
}

void GPK::setSize(Stream* stream, StreamSize newSize)
{
    return;
}

StreamSize GPK::read(Stream* stream, void* buf, StreamSize count)
{
#ifdef SD_DEBUG
    //printf("GPK: Position %li/%li (%li)\n", stream->m_data.position, stream->m_data.size, stream->m_data.offset + stream->m_data.position);
#endif
    stream->m_data.stream->seek(stream->m_data.offset + stream->m_data.position, spBegin);
    if (count > (stream->m_data.size - stream->m_data.position))
        count = stream->m_data.size - stream->m_data.position;
    StreamSize ret = stream->m_data.stream->read(buf, count);
    seek(stream, ret, spCurrent);
    return ret;
}

StreamSize GPK::write(Stream* stream, const void* buf, StreamSize count)
{
    return 0;
}

void GPK::close(Stream* stream)
{
    delete stream->m_data.stream;
}
