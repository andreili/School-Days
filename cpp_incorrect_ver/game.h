#ifndef _GAME_H_
#define _GAME_H_

#include "defines.h"
extern "C"
{
    #include <libavformat/avformat.h>
}

class Game
{
private:
    void start();
public:
    Game();
    ~Game();

    bool init();

    const char *get_settings(const char *name);
    bool load_tex(const char *name, int idx);
};

#endif
