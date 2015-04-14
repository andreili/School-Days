#include "defines.h"
#include "sound.h"
#include "fs.h"
#include "settings.h"
#include <string.h>
#include <map>
#include <string>

Sound::Sound()
{
    this->m_system = new SoundStreamP[SND_SIZE];
    memset(this->m_system, 0, sizeof(SoundStreamP) * SND_SIZE);
    this->m_system_bgm = -1;
}

Sound::~Sound()
{
#ifdef SD_DEBUG
    printf("Sound: free\n");
#endif
    this->clean();
    for (int i=0 ; i<SND_SIZE ; i++)
        if (this->m_system[i])
        {
            this->m_system[i]->stop();
            delete this->m_system[i];
        }
    delete this->m_system;

    // Выключаем текущий контекст
    alcMakeContextCurrent(NULL);
    // Уничтожаем контекст
    alcDestroyContext(this->m_context);
    // Закрываем звуковое устройство
    alcCloseDevice(this->m_device);
}

void Sound::clean()
{
    if (this->m_streams.size())
    {
        streams_t::iterator end = this->m_streams.end();
        for (streams_t::iterator it=this->m_streams.begin() ; it<end ; it++)
        {
            (*it)->stop();
            delete *it;
        }
    }
}

ALboolean Sound::CheckALCError()
{
    ALenum ErrCode;
    string Err = "ALC error: ";
    if ((ErrCode = alcGetError(this->m_device)) != ALC_NO_ERROR)
    {
        Err += (char *)alcGetString(this->m_device, ErrCode);
        ERROR_MESSAGEex(Err.c_str());
        return AL_FALSE;
    }
    return AL_TRUE;
}

bool Sound::init()
{
    // Позиция слушателя.
    ALfloat ListenerPos[] = { 0.0, 0.0, 0.0 };

    // Скорость слушателя.
    ALfloat ListenerVel[] = { 0.0, 0.0, 0.0 };

    // Ориентация слушателя. (Первые 3 элемента – направление «на», последние 3 – «вверх»)
    ALfloat ListenerOri[] = { 0.0, 0.0, -1.0,  0.0, 1.0, 0.0 };

    // Открываем заданное по умолчанию устройство
    this->m_device = alcOpenDevice(NULL);
    // Проверка на ошибки
    if (!this->m_device)
    {
        ERROR_MESSAGE("Sound: Default sound device not present");
        return false;
    }
    // Создаем контекст рендеринга
    this->m_context = alcCreateContext(this->m_device, NULL);
    if (!CheckALCError())
        return false;

    // Делаем контекст текущим
    alcMakeContextCurrent(this->m_context);

    // Устанавливаем параметры слушателя
    // Позиция
    alListenerfv(AL_POSITION,    ListenerPos);
    // Скорость
    alListenerfv(AL_VELOCITY,    ListenerVel);
    // Ориентация
    alListenerfv(AL_ORIENTATION, ListenerOri);

    load_system_snd();

    return true;
}

SoundStream *Sound::load_snd(const char *file_name, bool looped)
{
    char fn[MAX_PATH];
    strcpy(fn, file_name);
    if (looped)
        strcat(fn, "_loop");

    Stream *file = global.fs->open(fn);
    if (file->getFileStreamHandle() == INVALID_HANDLE_VALUE)
    {
        ERROR_MESSAGE("Sound: Sound %s don\'t open file\n", file_name);
        delete file;
        return NULL;
    }

    SoundStream *stream = new SoundStream();

    if (stream->load(file, looped))
    {
        NORMA_MESSAGE("Sound: Sound %s opened\n", file_name);
    }
    else
    {
        ERROR_MESSAGE("Sound: Sound %s don\'t open stream\n", file_name);
        delete file;
        delete stream;
        return NULL;
    }
    delete file;

    return stream;
}

void Sound::load_system_snd()
{
    this->m_system[SND_TITLE] = load_snd(global.settings->get_string("TitleBGM"), true);
    this->m_system[SND_SYSTEM] = load_snd(global.settings->get_string("SystemBGM"), true);
    this->m_system[SND_CANCEL] = load_snd(global.settings->get_string("SeCancel"), false);
    this->m_system[SND_SELECT] = load_snd(global.settings->get_string("SeSelect"), false);
    this->m_system[SND_CLICK] = load_snd(global.settings->get_string("SeClick"), false);
    this->m_system[SND_UP] = load_snd(global.settings->get_string("SeUp"), false);
    this->m_system[SND_DOWN] = load_snd(global.settings->get_string("SeDown"), false);
    this->m_system[SND_VIEW] = load_snd(global.settings->get_string("SeView"), false);
    this->m_system[SND_OPEN] = load_snd(global.settings->get_string("SeOpen"), false);
}

void Sound::play_system(int idx)
{
    if (this->m_system[idx])
        this->m_system[idx]->play();
}

void Sound::stop_system(int idx)
{
    if (this->m_system[idx])
        this->m_system[idx]->stop();
}

void Sound::play_system_bgm(int idx)
{
    if (this->m_system_bgm != idx)
    {
        if (this->m_system_bgm != -1)
            if (this->m_system[this->m_system_bgm])
                this->m_system[this->m_system_bgm]->stop();
        if (this->m_system[idx])
            this->m_system[idx]->play();
        this->m_system_bgm = idx;
    }
}

void Sound::stop_system_bgm(int idx)
{
    if (this->m_system_bgm != -1)
    {
        if (this->m_system[idx])
            this->m_system[idx]->stop();
        this->m_system_bgm = -1;
    }
}

SoundStream *Sound::add_snd(const char *file_name, int channel_idx, bool looped)
{
    SoundStream *stream = load_snd(file_name, looped);

    if (stream)
    {
        NORMA_MESSAGE("Sound: Sound %s opened\n", file_name);
        //stream->play();
        this->m_streams.push_back(stream);
        return stream;
    }
    else
    {
        return NULL;
    }
}
