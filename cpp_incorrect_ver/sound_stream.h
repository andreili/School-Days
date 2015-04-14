#ifndef _SOUND_STREAM_H_
#define _SOUND_STREAM_H_

#define AL_LIBTYPE_STATIC

#include "stream.h"
#include <AL/al.h>
#include <AL/alc.h>
//#include <AL/alu.h>
//#include <AL/alut.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>
#include <map>

using namespace std;

typedef struct
{
    unsigned int	ID;
    unsigned int	Rate;
    unsigned int	Format;
} SndInfo;

typedef map<ALuint, SndInfo> TBuf;

class SoundStream
{
private:
    ALuint          m_sourceID;
    ALint           m_proceed;
    OggVorbis_File  *m_VF;
    vorbis_comment	*m_comment;
    vorbis_info		*m_info;
    bool            m_loop;
    TBuf		    m_buffers;

    ALboolean CheckALError();
    bool load_ogg(Stream *stream);
    bool ReadOggBlock(ALuint BufID, size_t Size);
public:
    SoundStream();
    ~SoundStream();

    bool load(Stream *stream, bool looped);
    void play();
    void stop();
    void close();
};

#endif
