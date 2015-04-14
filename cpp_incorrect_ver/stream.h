#ifndef _STREAM_H_
#define _STREAM_H_

#include "defines.h"
#include <inttypes.h>
#include <vector>
#ifdef _WIN32
#include <windows.h>
typedef HANDLE fileHandle;
#define PATH_DELIMITER '\\'
#define PATH_DELIMITER_STR "\\"
#elif defined __unix__
#include <sys/types.h>
#include <malloc.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
typedef int fileHandle;
#define PATH_DELIMITER '/'
#define PATH_DELIMITER_STR "/"
#define INVALID_HANDLE_VALUE -1
#endif

class Stream;

#ifdef WIN32
#define FILE_OPEN_READ          0x80000000
#define FILE_OPEN_WRITE         0x40000000
#define FILE_OPEN_READWRITE     0xC0000000

#define FILE_SHARE_EXCLUSIVE    0x00
#define FILE_SHARE_DENY_WRITE   0x01
#define FILE_SHARE_DENY_READ    0x02
#define FILE_SHARE_DENY_NONE    0x03

#define FILE_CREATE_NEW         0x100
#define FILE_CREATE_ALWAYS      0x200

#define FILE_OPEN_EXISTING      0x300
#define FILE_OPEN_ALWAYS        0x400
#define FILE_TRUNCATE_EXISTING  0x500
#elif defined __unix__
#define FILE_OPEN_READ          O_RDONLY
#define FILE_OPEN_WRITE         O_WRONLY | S_IRWXU
#define FILE_OPEN_READWRITE     O_RDWR

#define FILE_SHARE_EXCLUSIVE    0x10
#define FILE_SHARE_DENY_WRITE   0x20
#define FILE_SHARE_DENY_READ    0       // not supported
#define FILE_SHARE_DENY_NONE    0x30

#define FILE_CREATE_NEW         O_CREAT | O_TRUNC
#define FILE_CREATE_ALWAYS      O_CREAT

#define FILE_OPEN_EXISTING      0       // not supported
#define FILE_OPEN_ALWAYS        O_CREAT
#define FILE_TRUNCATE_EXISTING  O_TRUNC
#endif
#define FILE_OPEN_READ_ST       FILE_OPEN_READ | FILE_SHARE_DENY_WRITE | FILE_OPEN_EXISTING
#define FILE_OPEN_WRITE_ST      FILE_OPEN_WRITE | FILE_SHARE_DENY_NONE | FILE_CREATE_ALWAYS
#define FILE_OPEN_READWRITE_ST  FILE_OPEN_READWRITE | FILE_SHARE_DENY_NONE | FILE_OPEN_EXISTING

typedef int64_t StreamSize;

enum ESeekMethod
{
    spBegin,
    spCurrent,
    spEnd
};

struct StreamData
{
    bool isChange;
    bool isExMem;
    fileHandle handle;
    StreamSize capacity;
    StreamSize size;
    StreamSize position;
    StreamSize offset;
    void* package;
    Stream* stream;
    std::vector<uint32_t> sectorsTable;
};

class StreamMethods
{
public:
    virtual StreamSize seek(Stream* stream, StreamSize moveTo, ESeekMethod method) = 0;
    virtual StreamSize getSize(Stream* stream) = 0;
    virtual void setSize(Stream* stream, StreamSize newSize) = 0;
    virtual StreamSize read(Stream* stream, void* buf, StreamSize count) = 0;
    virtual StreamSize write(Stream* stream, const void* buf, StreamSize count) = 0;
    virtual void close(Stream* stream) = 0;
};

class Stream
{
public:
    StreamMethods *m_methods;
    uint8_t* m_memory;
public:
    // Stream-on-Stream
    Stream(StreamMethods* methods);
    // file handled Stream
    Stream(const char* fileName, const uint32_t mode);
    // memory Stream
    Stream();
    Stream(StreamSize streamSize);
    // memoryEx Stream
    Stream(void* data, StreamSize size);
    ~Stream();
public:
    StreamData m_data;
public:
    fileHandle getFileStreamHandle();

    void* getMemory();

    StreamMethods* getMethods();

    StreamSize read(void* buf, StreamSize count);
    char* readStrZ();
    char* readStrLen(int32_t len);

    StreamSize write(const void* buf, StreamSize count);
    StreamSize writeStr(const char* str);

    StreamSize seek(StreamSize moveTo, ESeekMethod moveMethod);

    StreamSize getSize();
    void setSize(StreamSize newSize);

    StreamSize getPosition();
    void setPosition(StreamSize newPosition);

    StreamSize getCapacity();
    void SetCapacity(const StreamSize value);
};

#endif
