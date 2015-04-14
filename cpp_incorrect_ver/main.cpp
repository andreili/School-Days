#include "defines.h"
#include "game.h"
#include "interface.h"
#include <stdio.h>
#include <locale.h>

// Linux: X11, OpenGL, OpenAL, PThread

global_t global;

int main()
{
    //setlocale (LC_ALL,"ru_RU.utf8");
    setlocale(LC_ALL|~LC_NUMERIC, "");
    global.game = new Game();
    if (!global.game->init())
    {
    }
    delete global.game;
#ifdef SD_DEBUG
    printf("Exiting\n");
#endif
    return 0;
}
