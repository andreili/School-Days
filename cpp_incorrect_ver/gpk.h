#ifndef _GPK_H_
#define _GPK_H_

#include "defines.h"
#include "stream.h"
#include <stdlib.h>
#include <inttypes.h>
#include <vector>
#include <string>

using namespace std;

const unsigned char CIPHERCODE[16] = {0x82, 0xEE, 0x1D, 0xB3,
                                      0x57, 0xE9, 0x2C, 0xC2,
                                      0x2F, 0x54, 0x7B, 0x10,
                                      0x4C, 0x9A, 0x75, 0x49
                                     };

#define GPK_TAILER_IDENT0	"STKFile0PIDX"
#define GPK_TAILER_IDENT1	"STKFile0PACKFILE"

#pragma pack (1)
typedef struct
{
    char     sig0[12];
    uint32_t pidx_length;
    char     sig1[16];
} GPK_sig_t;

// gpk pidx format:
typedef struct
{
    uint16_t sub_version;		// same as script.gpk.* suffix
    uint16_t version;			// major version(always 1)
    uint16_t zero;			// always 0
    uint32_t offset;			// pidx data file offset
    uint32_t comprlen;			// compressed pidx data length
    char     dflt[4];			// magic "DFLT" or "    "
    uint32_t uncomprlen;		// raw pidx data length(if magic isn't DFLT, then this filed always zero)
    char     comprheadlen;		// pidx data header length
} GPK_pidx_t;

typedef struct
{
    char* name;
    GPK_pidx_t header;
} GPK_entry_t;
#pragma pack ()

typedef vector<GPK_entry_t*> GPK_entries_t;

class GPK : public StreamMethods
{
private:
    uint32_t        m_entries_count;
    GPK_entries_t   m_entries;
    char            m_file_name[MAX_PATH];

    void decode(char* buf, int size);
public:
    GPK();
    virtual ~GPK();

    bool load(const char* file_name);
    const char *get_file_name();

    GPK_pidx_t *get_entry(const char *file_name);

    void extract(const char *dir);
public:
    StreamSize seek(Stream* stream, StreamSize moveTo, ESeekMethod method);
    StreamSize getSize(Stream* stream);
    void setSize(Stream* stream, StreamSize newSize);
    StreamSize read(Stream* stream, void* buf, StreamSize count);
    StreamSize write(Stream* stream, const void* buf, StreamSize count);
    void close(Stream* stream);
};

#endif
