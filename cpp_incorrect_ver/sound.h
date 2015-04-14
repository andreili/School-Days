#ifndef _SOUND_H_
#define _SOUND_H_

#define AL_LIBTYPE_STATIC

#include "sound_stream.h"
#include <AL/al.h>
#include <AL/alc.h>
//#include <AL/alu.h>
//#include <AL/alut.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>
#include <vector>

#define SND_TITLE 0
#define SND_SYSTEM 1
#define SND_CANCEL 2
#define SND_SELECT 3
#define SND_CLICK 4
#define SND_UP 5
#define SND_DOWN 6
#define SND_VIEW 7
#define SND_OPEN 8

#define SND_SIZE 9

using namespace std;

typedef vector<SoundStream*> streams_t;
typedef SoundStream* SoundStreamP;

class Sound
{
private:
    streams_t   m_streams;
    ALCdevice   *m_device;
    ALCcontext  *m_context;
    SoundStreamP*m_system;
    int         m_system_bgm;

    ALboolean CheckALCError();

    SoundStream *load_snd(const char *file_name, bool looped);
    void load_system_snd();
public:
    Sound();
    ~Sound();

    bool init();
    void clean();

    void play_system(int idx);
    void stop_system(int idx);
    void play_system_bgm(int idx);
    void stop_system_bgm(int idx);

    SoundStream *add_snd(const char *file_name, int channel_idx, bool looped);
};

#endif
