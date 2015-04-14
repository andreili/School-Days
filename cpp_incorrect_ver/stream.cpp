#include "stream.h"

fileHandle inline fileCreate(const char* fileName, const uint32_t mode)
{
#ifdef WIN32
    uint32_t attr = (mode >> 16) & 0x1fff;
    if (attr == 0)
        attr = FILE_ATTRIBUTE_NORMAL;
    return CreateFile(fileName, mode & 0xF0000000, mode & 0xF, NULL, (mode >> 8) & 0xF, attr, 0);
#elif defined __unix__
    return open(fileName, mode);
#endif
}

StreamSize inline fileSeek(fileHandle handle, StreamSize moveTo, ESeekMethod method)
{
#ifdef WIN32
    uint32_t hiPtr = moveTo >> 32;
    uint32_t res = SetFilePointer(handle, uint32_t(moveTo), (LONG*)&hiPtr, method);
    if ((res == 0xFFFFFFFF) && (GetLastError() != NO_ERROR))
        return -1;
    else
        return (res | ((uint64_t)hiPtr << 32));
#elif defined __unix__
    return lseek(handle, moveTo, method);
#endif
}

StreamSize inline fileRead(fileHandle handle, void* buf, StreamSize count)
{
#ifdef WIN32
    uint32_t res = 0;
    if (!ReadFile(handle, buf, count, (DWORD*)&res, NULL))
        return 0;
#elif defined __unix__
    int32_t res = read(handle, buf, count);
    if (res == -1)
        return 0;
#endif
    return res;
}

StreamSize inline fileWrite(fileHandle handle, const void* buf, StreamSize count)
{
#ifdef WIN32
    uint32_t res = 0;
    if (!WriteFile(handle, buf, count, (DWORD*)&res, NULL))
        return 0;
#elif defined __unix__
    uint32_t res = write(handle, buf, count);
    if (res == (uint32_t)-1)
        return 0;
#endif
    return res;
}

void inline fileClose(fileHandle handle)
{
#ifdef WIN32
    CloseHandle(handle);
#elif defined __unix__
    close(handle);
#endif
}

////////////////////////////////////////////////////////////////////////////////
//                           BaseFileMethods                                  //
////////////////////////////////////////////////////////////////////////////////

class BaseFileMethods : public StreamMethods
{
public:
    StreamSize seek(Stream* stream, StreamSize moveTo, ESeekMethod method)
    {
        return fileSeek(stream->m_data.handle, moveTo, method);
    };
    StreamSize getSize(Stream* stream)
    {
#ifdef WIN32
        uint32_t sizeHi, res = GetFileSize(stream->m_data.handle, (DWORD*)&sizeHi);
        return (res | ((uint64_t)sizeHi << 32));
#elif defined __unix__
        StreamSize pos = stream->getPosition();
        StreamSize res = stream->seek(0, spEnd);
        stream->seek(pos, spBegin);
        return res;
#endif
    }
    void setSize(Stream* stream, StreamSize newSize)
    {
        uint32_t p = stream->getPosition();
        stream->setPosition(newSize);
#ifdef WIN32
        SetEndOfFile(stream->getFileStreamHandle());
#elif defined __unix__
#endif
        if (p < newSize)
            stream->setPosition(p);
    }
    StreamSize read(Stream* stream, void* buf, StreamSize count)
    {
        return fileRead(stream->m_data.handle, buf, count);
    }
    StreamSize write(Stream* stream, const void* buf, StreamSize count)
    {
        return fileWrite(stream->m_data.handle, buf, count);
    }
    void close(Stream* stream)
    {
        fileClose(stream->m_data.handle);
        stream->m_data.handle = INVALID_HANDLE_VALUE;
    }
};

static StreamMethods *baseFileMethods = new BaseFileMethods();

////////////////////////////////////////////////////////////////////////////////
//                            MemoryMethods                                   //
////////////////////////////////////////////////////////////////////////////////

class BaseMemoryMethods : public StreamMethods
{
public:
    virtual StreamSize seek(Stream* stream, StreamSize moveTo, ESeekMethod method)
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
    virtual StreamSize getSize(Stream* stream)
    {
        return stream->m_data.size;
    }
    virtual void setSize(Stream* stream, StreamSize newSize)
    {
        if (stream->m_data.capacity < newSize)
        {
            StreamSize newCap = newSize;
            if (stream->m_memory == NULL)
            {
                if (newSize != 0)
                    stream->m_memory = new uint8_t[newCap];
            }
            else
                stream->m_memory = (uint8_t*)realloc(stream->m_memory, newCap);
            stream->m_data.capacity = newCap;
        }
        else
        {
            if ((newSize == 0) && (stream->getSize() > 0) && (stream->m_memory != NULL))
            {
                delete (char*)stream->m_memory;
                stream->m_memory = NULL;
                stream->m_data.capacity = 0;
            }
        }
        stream->m_data.size = newSize;
        if (stream->m_data.position > stream->m_data.size)
            stream->m_data.position = stream->m_data.size;
    }
    virtual StreamSize read(Stream* stream, void* buf, StreamSize count)
    {
        if ((stream->m_data.position + count) > stream->m_data.size)
            count = stream->m_data.size - stream->m_data.position;
        buf = memcpy(buf, (void*)(stream->m_memory + stream->m_data.position), count);
        stream->m_data.position += count;
        return count;
    }
    virtual StreamSize write(Stream* stream, const void* buf, StreamSize count)
    {
        if ((count + stream->m_data.position) > stream->m_data.size)
            stream->setSize(stream->m_data.position + count);
        memcpy((void*)(stream->m_memory + stream->m_data.position), buf, count);
        stream->m_data.position += count;
        return count;
    }
    virtual void close(Stream* stream)
    {
        if ((stream->m_memory != NULL) && (!stream->m_data.isExMem))
        {

            delete (char*)stream->m_memory;
            stream->m_memory = NULL;
        }
    }
};

static StreamMethods* baseMemoryMethods = new BaseMemoryMethods();

////////////////////////////////////////////////////////////////////////////////
//                                TStream                                     //
////////////////////////////////////////////////////////////////////////////////

Stream::Stream(StreamMethods* methods)
{
    this->m_methods = methods;
    this->m_data.isExMem = false;
    this->m_data.position = 0;
}

Stream::Stream(const char* fileName, const uint32_t mode)
{
    if ((mode & FILE_OPEN_READWRITE) == FILE_OPEN_READWRITE)
    {
        //if (ex)
        this->m_data.isExMem = false;
    }
    this->m_methods = baseFileMethods;

    this->m_memory = NULL;
    memset(&this->m_data, 0, sizeof(StreamData));
    this->m_data.handle = fileCreate(fileName, mode);
    this->m_data.isChange = false;
    this->m_data.isExMem = false;
}

Stream::Stream()
{
    this->m_methods = baseMemoryMethods;

    this->m_memory = NULL;
    memset(&this->m_data, 0, sizeof(StreamData));
    this->m_data.isChange = false;
    this->m_data.isExMem = false;
}

Stream::Stream(StreamSize streamSize)
{
    this->m_methods = baseMemoryMethods;

    this->m_memory = NULL;
    memset(&this->m_data, 0, sizeof(StreamData));
    this->m_data.isChange = false;
    this->m_data.isExMem = false;
    this->setSize(streamSize);
}

Stream::Stream(void*  data, StreamSize size)
{
    this->m_methods = baseMemoryMethods;

    memset(&this->m_data, 0, sizeof(StreamData));
    this->m_memory = (uint8_t*)data;
    this->m_data.size = size;
    this->m_data.isChange = false;
    this->m_data.isExMem = true;
}

Stream::~Stream()
{
    this->seek(0, spBegin);
    this->m_methods->close(this);
}

fileHandle Stream::getFileStreamHandle()
{
    return this->m_data.handle;
}

void* Stream::getMemory()
{
    return this->m_memory;
}

StreamMethods* Stream::getMethods()
{
    return this->m_methods;
}

StreamSize Stream::read(void* buf, StreamSize count)
{
    return this->m_methods->read(this, buf, count);
}

char* Stream::readStrZ()
{
    char c;
    uint32_t startPos = this->getPosition();
    do
    {
        if (this->read(&c, 1) == 0)
            break;
    }
    while (c != '\x00');
    int32_t length = this->getPosition() - startPos;
    this->seek(startPos, spBegin);
    return this->readStrLen(length);
}

char* Stream::readStrLen(int32_t len)
{
    char* str = new char[len+1];
    this->read(str, len);
    str[len] = '\x00';
    return str;
}

StreamSize Stream::write(const void* buf, StreamSize count)
{
    return this->m_methods->write(this, buf, count);
}

StreamSize Stream::writeStr(const char* str)
{
    return this->write(str, strlen(str));
}

StreamSize Stream::seek(StreamSize moveTo, ESeekMethod moveMethod)
{
    return this->m_methods->seek(this, moveTo, moveMethod);
}

StreamSize Stream::getSize()
{
    return this->m_methods->getSize(this);
}

void Stream::setSize(StreamSize newSize)
{
    this->m_methods->setSize(this, newSize);
}

StreamSize Stream::getPosition()
{
    return this->seek(0, spCurrent);
}

void Stream::setPosition(StreamSize newPosition)
{
    this->m_methods->seek(this, newPosition, spBegin);
}

StreamSize Stream::getCapacity()
{
    return this->m_data.capacity;
}

void Stream::SetCapacity(const StreamSize value)
{
    StreamSize v, oldSize;
    v = value;
    if (value < this->m_data.size)
    {
        v = this->m_data.size;
    }
    if (value > this->m_data.capacity)
    {
        oldSize = this->getSize();
        this->setSize(v);
        this->setSize(oldSize);
    }
    else
    {
        if (this->m_memory != NULL)
        {
            this->m_memory = (uint8_t*)realloc(this->m_memory, v);
            this->m_data.capacity = v;
        }
    }
}
