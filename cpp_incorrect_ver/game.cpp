#include "defines.h"
#include "game.h"
#include "fs.h"
#include "settings.h"
#include "interface.h"
#include "script_engine.h"
#include "sound.h"
#include "video.h"
#include "parser.h"
#include "menu.h"

#define SD_PACKS_COUNT 30
const char* SD_PACKS[] = {"Ini", "System", "Script", "SysSe", "BGM",
                          "Event00", "Event01", "Event02", "Event03", "Event04", "Event05",
                          "Movie00", "Movie01", "Movie02", "Movie03", "Movie04", "Movie05",
                          "Se00", "Se01", "Se02", "Se03", "Se04", "Se05",
                          "Voice00", "Voice01", "Voice02", "Voice03", "Voice04", "Voice05",
                          "Commentary"
                         };

Game::Game()
{
    global.fs = NULL;
    global._interface = NULL;
    global.settings = NULL;
    global.engine = NULL;
    global.sound = NULL;
}

Game::~Game()
{
#ifdef SD_DEBUG
    printf("Game: free\n");
#endif
    if (global.menu)
        delete global.menu;
    if (global.engine)
        delete global.engine;
    if (global.fs)
        delete global.fs;
    if (global._interface)
        delete global._interface;
    if (global.settings)
        delete global.settings;
    if (global.sound)
        delete global.sound;
    if (global.video)
        delete global.video;
}

bool Game::init()
{
    global.fs = new FS("./");

    global.settings = new Settings();
    global.settings->load("game.ini");
    global.fs->set_ext(global.settings->get_string("FileExtend"));
    global.settings->load(global.settings->get_string("DXGraphicBase"));
    global.settings->load(global.settings->get_string("DXSoundBase"));
    global.settings->load(global.settings->get_string("FILMEngine"));
    global.settings->load(global.settings->get_string("StartScript"));
    global.settings->load(global.settings->get_string("DebugInfo"));
    global.settings->load(global.settings->get_string("EndingList"));
    global.settings->load(global.settings->get_string("Dummy"));
    global.settings->load(global.settings->get_string("ConfigFile"));

    global._interface = new Interface();
    int w = atoi(global.settings->get_string("WindowWidth"));
    int h = atoi(global.settings->get_string("WindowHeight"));
    if (!w)
    {
        w = atoi(global.settings->get_string("Width"));
        h = atoi(global.settings->get_string("Height"));
    }
    if (!global._interface->init(global.settings->get_string("Name"), w, h, false))
    {
        ERROR_MESSAGE("Game: Doesn\'t init iterface\n");
        return false;
    }

    for (int i=0 ; i<SD_PACKS_COUNT ; i++)
    {
        if (!global.fs->mount_pack(SD_PACKS[i]))
        {
            ERROR_MESSAGE("Game: pack %s not exist\n", SD_PACKS[i]);
        }
    }

    global.engine = new ScriptEngine();

    global.sound = new Sound();
    if (!global.sound->init())
    {
        ERROR_MESSAGE("Game: Doesn\'t init sound\n");
        return false;
    }

    av_register_all();
    if (!global.video->init())
    {
        ERROR_MESSAGE("Game: Doesn\'t init video\n");
        return false;
    }

    global.menu = new Menu();

    start();

    global._interface->wnd_proc();
    return true;
}

void Game::start()
{
    //load_tex(global.settings->get_value("SystemBase"), LAYER_SYSBASE);
    load_tex(global.settings->get_string("BaseFile"), LAYER_TITLE_BASE);
}

bool Game::load_tex(const char *name, int idx)
{
    return global._interface->load_tex(name, idx);
}

const char *Game::get_settings(const char *name)
{
    return global.settings->get_string(name);
}
