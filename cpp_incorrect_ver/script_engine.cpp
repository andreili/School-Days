#include "defines.h"
#include "game.h"
#include "script_engine.h"
#include "interface.h"
#include "sound.h"
#include "sound_stream.h"
#include "settings.h"
#include "video.h"
#include "menu.h"
#include "fs.h"
#include <math.h>
#include <string.h>

uint32_t ScriptEngine::get_time_ms()
{
    timeval tm;
    timerclear(&tm);
    gettimeofday(&tm, NULL);
    return (tm.tv_sec * 1000) + tm.tv_usec / 1000;
}

ScriptEngine::ScriptEngine()
{
    this->m_time = get_time_ms();
    this->m_run = false;
    this->m_events.clear();
}

ScriptEngine::~ScriptEngine()
{
#ifdef SD_DEBUG
    printf("ScriptEngine: free\n");
#endif
}

void ScriptEngine::events_proc()
{
    if (!this->m_run)
        return;

    this->m_time = get_time_ms();
    //NORMA_MESSAGE("ScriptEngine: %i\n", this->m_time - this->m_start);
    if (this->m_events.size())
    {
        events_t::iterator end = this->m_events.end();
        for (events_t::iterator it=this->m_events.begin() ; it<end ; it++)
        {
            event_t *event = *it;
            //Video *video;

            if ((event->state == EVENT_WAIT) && (event->t_start <= this->m_time))
            {
                NORMA_MESSAGE("ScriptEngine: Event '%s' start at %i\n", event->data[EVENT_FIED_FILE], this->m_time - this->m_start);
                switch (event->type)
                {
                    case EVENT_BLACK_FADE:
                        global._interface->set_fade_color(0);
                        event->dir = ((strcmp(event->data[EVENT_FIED_FILE], "IN") == 0) ? true : false);
                        if (event->dir)
                            event->f_val = 0.0;
                        else
                            event->f_val = 1.0;
                        global._interface->set_fade(event->f_val);
                        break;
                    case EVENT_WHITE_FADE:
                        global._interface->set_fade_color(1);
                        event->dir = ((strcmp(event->data[EVENT_FIED_FILE], "IN") == 0) ? true : false);
                        if (event->dir)
                            event->f_val = 0.0;
                        else
                            event->f_val = 1.0;
                        global._interface->set_fade(event->f_val);
                        break;
                    case EVENT_BG:
                        global._interface->load_tex(event->data[EVENT_FIED_FILE], LAYER_BG);
                        break;
                    case EVENT_MOVIE:
                        //video = new Video();

                        //event->mem = video;
                        break;
                    case EVENT_SE:
                        break;
                    case EVENT_TEXT:
                        break;
                    case EVENT_BGM:
                        ((SoundStream*)event->mem)->play();
                        break;
                    case EVENT_VOICE:
                        break;
                    case EVENT_NONE:
                        break;
                }

                event->t_delta = event->t_end - event->t_start;
                event->state = EVENT_RUN;

                //break;
            }
            else if ((event->state == EVENT_RUN) && (event->t_end != 0) && (event->t_end <= this->m_time))
            {
                NORMA_MESSAGE("ScriptEngine: Event '%s' end at %i\n", event->data[EVENT_FIED_FILE], this->m_time - this->m_start);
                switch (event->type)
                {
                    case EVENT_BLACK_FADE:
                    case EVENT_WHITE_FADE:
                        if (event->dir)
                            event->f_val = 1.0;
                        else
                            event->f_val = 0.0;
                        global._interface->set_fade(event->f_val);
                        global._interface->set_fade_color(-1);
                        break;
                    case EVENT_BG:
                        global._interface->unload_tex(LAYER_BG);
                        break;
                    case EVENT_MOVIE:
                        break;
                    case EVENT_SE:
                        break;
                    case EVENT_TEXT:
                        break;
                    case EVENT_BGM:
                        break;
                    case EVENT_VOICE:
                        break;
                    case EVENT_NONE:
                        break;
                }
                event->state = EVENT_END;
                if (event->next_state)
                {
                    global.menu->next_state(-1);
                    //break;
                }

                //break;
            }
            else if (event->state == EVENT_RUN)
            {
                switch (event->type)
                {
                    case EVENT_BLACK_FADE:
                    case EVENT_WHITE_FADE:
                        if (event->dir)
                            event->f_val = (this->m_time - event->t_start * 1.) / event->t_delta;
                            //event->f_val = exp(-7. * ((event->t_end - time) / event->t_delta));
                        else
                            event->f_val = (event->t_end - this->m_time * 1.) / event->t_delta;
                            //event->f_val = exp(-2. * ((this->m_time - event->t_start * 1.) / event->t_delta));
                        global._interface->set_fade(event->f_val);
                        break;
                    case EVENT_BG:
                        break;
                    case EVENT_MOVIE:
                        break;
                    case EVENT_SE:
                        break;
                    case EVENT_TEXT:
                        break;
                    case EVENT_BGM:
                        break;
                    case EVENT_VOICE:
                        break;
                    case EVENT_NONE:
                        break;
                }

                //break;
            }
        }
    }

    //global.interface->draw();
}

void ScriptEngine::clear()
{
    if (this->m_events.size())
    {
        events_t::iterator end = this->m_events.end();
        for (events_t::iterator it=this->m_events.begin() ; it<end ; it++)
        {
            event_t *event = *it;
            switch (event->type)
            {
                case EVENT_BLACK_FADE:
                case EVENT_WHITE_FADE:
                    break;
                case EVENT_BG:
                    //global.game->load_tex(event->data[EVENT_FILE], LAYER_GAME);
                    break;
                case EVENT_MOVIE:
                    //video = new Video();

                    //event->mem = video;
                    break;
                case EVENT_SE:
                    break;
                case EVENT_TEXT:
                    break;
                case EVENT_BGM:
                    delete ((SoundStream*)event->mem);
                    break;
                case EVENT_VOICE:
                    break;
                case EVENT_NONE:
                    break;
            }
        }
    }
    this->m_events.clear();
}

void ScriptEngine::add_event(EEventType type, const char *data, const char *start, const char *end, bool next_state)
{
    NORMA_MESSAGE("ScriptEngine: Add event '%s' end at %s\n", data, start);
    event_t *event = new event_t;
    memset(event, 0, sizeof(event_t));
    event->type = type;
    strcpy(event->data[EVENT_FIED_FILE], data);
    strcpy(event->start, start);
    strcpy(event->end, end);
    event->next_state = next_state;
    this->m_events.push_back(event);
}

int strchr_pos(const char *str, const char chr)
{
    return strchr(str, chr) - str;
}

void set_field(event_t *event, int idx, const char **raw)
{
    int pos = strchr_pos(*raw, '\t');
    strncpy(event->data[idx], *raw, pos);
    *raw += pos + 1;
}

void ScriptEngine::parse(const char *file_name)
{
    this->clear();
    Stream *str = global.fs->open(file_name);
    if (str->getFileStreamHandle() == INVALID_HANDLE_VALUE)
    {
        ERROR_MESSAGE("ScriptEngine: File %s doesn\'t opened\n", file_name);
        return;
    }

    Parser *ini = new Parser(str);

    uint32_t count = ini->get_count();
    for (uint32_t i=0 ; i<count ; i++)
    {
        event_t *event = new event_t;
        memset(event, 0, sizeof(event_t));
        const char *type = ini->get_name(i);
        const char *raw = ini->get_value(i);
        int pos;

        strncpy(event->start, raw, 8);
        raw += 9;

        if (strcmp(type, "CreateBG") == 0)
        {
            pos = strchr_pos(raw, '\t');
            raw += pos + 1;

            set_field(event, EVENT_FIED_FILE, &raw);
            event->type = EVENT_BG;
        }
        else if (strcmp(type, "PlayMovie") == 0)
        {
            set_field(event, EVENT_FIED_FILE, &raw);
            set_field(event, EVENT_FIED_LAYER_IDX, &raw);
            event->type = EVENT_MOVIE;
        }
        else if (strcmp(type, "PlaySe") == 0)
        {
            set_field(event, EVENT_FIED_LAYER_IDX, &raw);
            set_field(event, EVENT_FIED_FILE, &raw);
            event->type = EVENT_SE;
        }
        else if (strcmp(type, "PrintText") == 0)
        {
            set_field(event, EVENT_FIED_TITLE, &raw);
            set_field(event, EVENT_FIED_TEXT, &raw);
            event->type = EVENT_TEXT;
        }
        else if (strcmp(type, "BlackFade") == 0)
        {
            set_field(event, EVENT_FIED_FILE, &raw);
            event->type = EVENT_BLACK_FADE;
        }
        else if (strcmp(type, "WhiteFade") == 0)
        {
            set_field(event, EVENT_FIED_FILE, &raw);
            event->type = EVENT_WHITE_FADE;
        }
        else if (strcmp(type, "PlayVoice") == 0)
        {
            set_field(event, EVENT_FIED_TITLE, &raw);
            set_field(event, EVENT_FIED_TEXT, &raw);
            event->type = EVENT_TEXT;
        }
        else if (strcmp(type, "PlayBgm") == 0)
        {
            set_field(event, EVENT_FIED_FILE, &raw);
            event->type = EVENT_BGM;
        }
        else if (strcmp(type, "PlayVoice") == 0)
        {
            set_field(event, EVENT_FIED_FILE, &raw);
            set_field(event, EVENT_FIED_LAYER_IDX, &raw);
            set_field(event, EVENT_FIED_PERS, &raw);
            event->type = EVENT_VOICE;
        }
        else ERROR_MESSAGE("ScriptEngine: Undefined event type - %s\n", type);

        if (raw[0] == ';')
            event->end[0] = '\0';
        else
            strncpy(event->end, raw, 8);
        this->m_events.push_back(event);
    }
}

uint32_t ScriptEngine::str2time(const char *time)
{
    uint32_t min = atoi(time);
    time += 3;
    uint32_t sec = atoi(time);
    time += 3;
    uint32_t usec = atoi(time);
    return this->m_start + (((min * 60) + sec) * 1000 + usec*16.6667);

}

void ScriptEngine::start()
{
    global.sound->clean();
    this->m_run = true;
    this->m_start = get_time_ms();
    events_t::const_iterator end = this->m_events.end();
    for (events_t::const_iterator it=this->m_events.begin() ; it<end ; it++)
    {
        event_t *event = *it;

        event->t_start = str2time(event->start);
        if (strlen(event->end) > 0)
            event->t_end = str2time(event->end);
        else
            event->t_end = 0;
        event->state = EVENT_WAIT;

        switch (event->type)
        {
            case EVENT_BLACK_FADE:
            case EVENT_WHITE_FADE:
                break;
            case EVENT_BG:
                //global.game->load_tex(event->data, LAYER_GAME);
                break;
            case EVENT_MOVIE:
                //video = new Video();

                //event->mem = video;
                break;
            case EVENT_SE:
                break;
            case EVENT_TEXT:
                break;
            case EVENT_BGM:
                event->mem = global.sound->add_snd(event->data[EVENT_FIED_FILE], 0, true);
                break;
            case EVENT_VOICE:
                break;
            case EVENT_NONE:
                break;
        }

        //NORMA_MESSAGE("ScriptEngine: Event '%s' offset %li sec length %li sec\n", event->data[EVENT_FIED_FILE], event->t_start - this->m_start.tv_sec, event->t_end - event->t_start);
    }
}

void ScriptEngine::stop()
{
    this->m_run = false;
    this->clear();
}

void ScriptEngine::pause(bool paused)
{
    if (paused)
    {
        this->m_pause_start = this->get_time_ms();
        this->m_run = true;
    }
    else
    {
        uint32_t cur_time = this->get_time_ms();
        this->m_start += cur_time + this->m_pause_start;
        this->m_run = false;
    }
}

void ScriptEngine::dump()
{
    events_t::const_iterator end = this->m_events.end();
    for (events_t::const_iterator it=this->m_events.begin() ; it<end ; it++)
    {
        event_t *event = *it;
        printf("Event start at %u end at %u\n", event->t_start, event->t_end);
    }
}
