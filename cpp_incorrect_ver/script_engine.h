#ifndef _SCRIPT_ENGINE_H_
#define _SCRIPT_ENGINE_H_

#include <sys/time.h>
#include <time.h>
#include <vector>

typedef enum
{
    EVENT_BG,       // complete?
    EVENT_MOVIE,
    EVENT_SE,
    EVENT_BLACK_FADE,     // complete
    EVENT_WHITE_FADE,     // complete
    EVENT_TEXT,
    EVENT_BGM,
    EVENT_VOICE,
    EVENT_NONE
} EEventType;

typedef enum
{
    EVENT_WAIT,
    EVENT_RUN,
    EVENT_END
} EEventState;

#define EVENT_FIED_FILE 0
#define EVENT_FIED_LAYER_IDX 1
#define EVENT_FIED_TITLE 2
#define EVENT_FIED_TEXT 3
#define EVENT_FIED_PERS 4
#define EVENT_EX_START 5

typedef struct
{
    EEventType  type;
    char  data[10][100];
    char  start[10];
    char  end[10];
    uint32_t    t_start;
    uint32_t    t_end;
    uint32_t    t_delta;
    bool        dir;
    float       f_val;
    void        *mem;
    bool        next_state;
    EEventState state;
} event_t;

typedef std::vector<event_t*> events_t;

class ScriptEngine
{
private:
    uint32_t    m_time;
    uint32_t    m_start;
    uint32_t    m_pause_start;
    events_t    m_events;

    bool        m_run;

    uint32_t str2time(const char *time);
    uint32_t get_time_ms();
public:
    ScriptEngine();
    ~ScriptEngine();

    void events_proc();
    void clear();

    void parse(const char *file_name);

    void add_event(EEventType type, const char *data, const char *start, const char *end, bool next_state);
    void start();
    void stop();
    void pause(bool paused);

    void dump();
};

#endif
