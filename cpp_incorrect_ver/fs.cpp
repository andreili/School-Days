#include "defines.h"
#include "fs.h"
#include "stream.h"
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>

#ifdef SD_DEBUG
#endif

FS::FS(const char *root_dir)
{
    strcpy(this->m_root, root_dir);
#ifdef SD_DEBUG
    char path[MAX_PATH];
#ifdef _WIN32
    _fullpath(path, root_dir, MAX_PATH);
#elif defined __unix__
    realpath(root_dir, path);
#else
    #error here!
#endif
    printf("FS: mounted to %s\n", path);
#endif
}

FS::~FS()
{
#ifdef SD_DEBUG
    printf("FS: free\n");
#endif
    uint32_t size = this->m_packs.size();
    for (uint32_t i=0 ; i<size ; i++)
    {
        delete this->m_packs.at(i);
    }
    this->m_packs.clear();
}

void FS::set_ext(const char *ext)
{
    strcpy(this->m_ext, ext);
}

bool FS::add_packs(const char *path)
{
    DIR *dir = opendir(path);
    if (dir)
    {
        struct dirent *ent;
        while((ent = readdir(dir)) != NULL)
            if (strstr(ent->d_name, this->m_ext))
            {
#ifdef SD_DEBUG
                printf("FS: pack %s add trying\n", ent->d_name);
#endif
                char fn[MAX_PATH];
                strcpy(fn, path);
                strcat(fn, ent->d_name);

                GPK *gpk = new GPK();
                if (!gpk->load(fn))
                {
                    delete gpk;
                    return false;
                }
                this->m_packs.push_back(gpk);
                NORMA_MESSAGE("FS: pack %s mounted\n", ent->d_name);
            }
    }

    return true;
}

bool FS::mount_pack(const char *file_name)
{
    char fn[MAX_PATH];
    strcpy(fn, PACKS_DIR);
    strcat(fn, file_name);
    strcat(fn, ".GPK");

    GPK *gpk = new GPK();
    if (!gpk->load(fn))
    {
        delete gpk;
        return false;
    }
    //gpk->extract("./extr/");
    this->m_packs.push_back(gpk);

    NORMA_MESSAGE("FS: pack %s mounted\n", file_name);
    return true;
}

Stream *FS::open(const char *file_name)
{
    char fn[MAX_PATH];
    strcpy(fn, file_name);
    #ifdef _WIN32
    int len = strlen(fn);
    for (int i=0 ; i<len ; i++)
        if (fn[i] == '/')
            fn[i] = PATH_DELIMITER;
    #endif

    struct stat sts;
    if (stat(fn, &sts) != -1)
    {
        return new Stream(fn, FILE_OPEN_READ_ST);
    }

    const char *_fn = strchr(file_name, '/') + 1;
    if ((strncmp(fn, "BGM", 3) == 0) || (strncmp(fn, "SysSe", 3) == 0))
    {
        char fn_1[MAX_PATH];
        strcpy(fn_1, _fn);
        strcat(fn_1, ".ogg");
        _fn = fn_1;
    }
    uint32_t size = this->m_packs.size();
    for (uint32_t i=0 ; i<size ; i++)
    {
        GPK_pidx_t* pidx = this->m_packs.at(i)->get_entry(_fn);
        if (pidx != NULL)
        {
            Stream *res = new Stream(this->m_packs.at(i));

            if (pidx->uncomprlen)
                res->m_data.size = pidx->uncomprlen;
            else
                res->m_data.size = pidx->comprlen;
            res->m_data.offset = pidx->offset;
            res->m_data.handle = (fileHandle)pidx->offset;
            res->m_data.stream = new Stream(this->m_packs.at(i)->get_file_name(), FILE_OPEN_READ_ST);
            NORMA_MESSAGE("FS: File %s opened. Size="PRId64"\n", _fn, res->m_data.size);

            return res;
        }
    }
    ERROR_MESSAGE("FS: File %s doesn\'t exist\n", _fn);
    Stream *str = new Stream();
    str->m_data.handle = INVALID_HANDLE_VALUE;
    return str;
}
