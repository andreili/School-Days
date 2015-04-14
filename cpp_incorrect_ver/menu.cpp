#include "menu.h"
#include "parser.h"
#include "sound.h"
#include "settings.h"
#include "interface.h"
#include "script_engine.h"
#include <stdlib.h>

#define SETTINGS_BGM_VOL "BgmVolume"
#define SETTINGS_SE_VOL "SeVolume"
#define SETTINGS_VO_VOL "VoiceVolume"
#define SETTINGS_MUTE "Mute"
#define SETTINGS_MEN_VO "MenVoice"

Menu::Menu()
{
    this->m_state = MENU_INIT;
    this->m_in_game = false;
    this->m_dlg_active = false;
    memset(this->m_dlg_regions, 0, sizeof(region_t) * 2);
    memset(this->m_dlg_chips, 0, sizeof(region_chip_t) * 2);
}

Menu::~Menu()
{
#ifdef SD_DEBUG
    printf("Menu: free\n");
#endif
    this->unload_menu();
}

bool Menu::region_check(region_t *region, CURSOR *pointer, int idx)
{
    if ((region->x1 <= pointer->x) && (region->x2 >= pointer->x) &&
        (region->y1 <= pointer->y) && (region->y2 >= pointer->y))
    {
        if (region->state == MENU_DEFAULT)
            region->state = MENU_MOUSE_OVER;
        else if (region->state == MENU_SELECTED)
            region->state = MENU_SELECTED_MOUSE;

        if (pointer->left_presed)
        {
            pointer->left_presed = false;
            this->next_state(idx);
            if (this->m_state == MENU_EXIT)
                return false;
            return true;
        }
    }
    else if (region->state == MENU_MOUSE_OVER)
        region->state = MENU_DEFAULT;
    else if (region->state == MENU_SELECTED_MOUSE)
        region->state = MENU_SELECTED;
    return true;
}

bool Menu::proc(CURSOR *pointer)
{
    if (pointer->rigth_presed)
    {
        pointer->rigth_presed = false;
        this->prev_state(-1);
        return true;
    }

    if (this->m_dlg_active)
    {
        if (!region_check(&this->m_dlg_regions[0], pointer, 0))
            return false;
        if (this->m_dlg_regions[0].state == this->m_dlg_chips[0].state)
            this->m_dlg_regions[0].chip = &this->m_dlg_chips[0];
        region_check(&this->m_dlg_regions[1], pointer, 1);
        if (this->m_dlg_regions[1].state == this->m_dlg_chips[1].state)
            this->m_dlg_regions[1].chip = &this->m_dlg_chips[1];
    }
        else
    {
        regions_t::iterator end = this->m_regions.end();
        for (regions_t::iterator it=this->m_regions.begin() ; it<end ; it++)
        {
            region_t *region = *it;
            if (region->state != MENU_DISABLE)
                if (!region_check(region, pointer, region->idx))
                    return false;
        }
        regions_chip_t::iterator chip_end = this->m_chips.end();
        for (regions_chip_t::iterator it=this->m_chips.begin() ; it<chip_end ; it++)
        {
            region_chip_t *chip = *it;
            if (chip->region > this->m_regions.size())
                continue;
            region_t *region = this->m_regions.at(chip->region - 1);
            if (region->state == chip->state)
            {
                region->chip = chip;
            }
        }
    }

    return true;
}

void Menu::prev_state(int idx)
{
    int state_last = this->m_state;
    switch (this->m_state)
    {
        case MENU_TITLE:
            break;
        case MENU_LOAD:
        case MENU_SETTINGS:
        case MENU_SETTINGS_SOUND:
            if (this->m_in_game)
            {
                ERROR_MESSAGE("Menu: Not supported\n");
            }
            else
            {
                this->m_next_state = MENU_TITLE;
                this->m_state = MENU_PRE_TITLE;
                this->menu_exit_effect();
                break;
                //this->m_state = MENU_TITLE;
                //show_title();
            }
            break;
        case MENU_REPLAY:
            this->m_state = MENU_TITLE;
            show_title();
            break;
        case MENU_EXIT_DLG:
            unload_dlg();
            if (this->m_in_game)
            {
            }
            else
                this->m_state = MENU_TITLE;
            break;
    }
    printf("Menu::prev_state(%i) last state %i new state %i\n", idx, state_last, this->m_state);
}

void Menu::next_state(int idx)
{
    int state_last = this->m_state;
    switch (this->m_state)
    {
        case MENU_INIT:
            this->m_state = MENU_SPLASH;
            show_splash();
            break;
        case MENU_SPLASH:
            this->m_state = MENU_TITLE;
            show_title();
            break;
        case MENU_TITLE:
            switch (idx)
            {
                case 0:
                    //this->m_state = MENU_NEW_GAME;
                    //global.sound->clean();
                    ERROR_MESSAGE("Menu: Not supported - new game\n");
                    break;
                case 1:
                    this->m_next_state = MENU_LOAD;
                    this->m_state = MENU_PRE_LOAD;
                    this->menu_exit_effect();
                    break;
                case 2:
                    //this->m_next_state = MENU_REPLAY;
                    //this->m_state = MENU_PRE_REPLAY;
                    //this->menu_exit_effect();
                    this->m_state = MENU_REPLAY;
                    ERROR_MESSAGE("Menu: Not supported - replay\n");
                    break;
                case 3:
                    this->m_next_state = MENU_SETTINGS;
                    this->m_state = MENU_PRE_SETTINGS;
                    this->menu_exit_effect();
                    break;
                case 4:
                    this->m_state = MENU_EXIT_DLG;
                    //this->m_state = MENU_EXIT;
                    show_exit_dlg();
                    ERROR_MESSAGE("Menu: Not supported - exit dlg\n");
                    break;
            }
            break;
        case MENU_PRE_TITLE:
            this->m_state = this->m_next_state;
            show_title();
            this->menu_start_effect();
            break;
        case MENU_PRE_LOAD:
            this->m_state = this->m_next_state;
            show_load();
            this->menu_start_effect();
            break;
        case MENU_PRE_SETTINGS:
            this->m_state = this->m_next_state;
            show_settings();
            this->menu_start_effect();
            break;
        case MENU_EXIT_DLG:
            switch (idx)
            {
                case 0:
                    this->m_state = MENU_EXIT;
                    break;
                case 1:
                    prev_state(-1);
                    break;
            }
            break;
        case MENU_LOAD:
            switch (idx)
            {
                case 1:
                case 2:
                case 3:
                case 4:
                case 5:
                case 6:
                case 7:
                case 8:
                case 9:
                case 10:
                case 11:
                case 12:
                case 13:
                case 14:
                case 15:
                case 16:
                case 17:
                case 18:
                case 19:
                    break;
                case 20:
                    this->prev_state(-1);
                    break;
                case 21:
                case 22:
                case 23:
                case 24:
                case 25:
                case 26:
                case 27:
                case 28:
                case 29:
                case 30:
                case 31:
                case 32:
                    break;
            }
            break;
        case MENU_SETTINGS:
            switch (idx)
            {
                case 0:
                    break;
                case 1:
                    this->m_state = MENU_SETTINGS_SOUND;
                    show_settings_snd();
                    break;
                case 2:
                    this->prev_state(-1);
                    break;
                case 3:
                case 4:
                case 5:
                case 6:
                case 7:
                case 8:
                case 9:
                case 10:
                case 11:
                case 12:
                    if (this->m_regions.at(idx)->state == MENU_SELECTED_MOUSE)
                        this->m_regions.at(idx)->state = MENU_DEFAULT;
                    else
                        this->m_regions.at(idx)->state = MENU_SELECTED;
                    break;
            }
            break;
        case MENU_SETTINGS_SOUND:
            switch (idx)
            {
                case 0:
                    this->m_state = MENU_SETTINGS;
                    show_settings();
                    break;
                case 1:
                    break;
                case 2:
                    this->prev_state(-1);
                    break;
                case 3:
                case 4:
                case 5:
                case 6:
                case 7:
                case 8:
                case 9:
                case 10:
                case 11:
                case 12:
                    break;
                case 13:
                case 14:
                case 15:
                case 16:
                case 17:
                case 18:
                case 19:
                case 20:
                case 21:
                case 22:
                    global.settings->set_int(SETTINGS_BGM_VOL, idx - 12);
                    this->read_state(SETTINGS_BGM_VOL, 13);
                    break;
                case 23:
                case 24:
                case 25:
                case 26:
                case 27:
                case 28:
                case 29:
                case 30:
                case 31:
                case 32:
                    global.settings->set_int(SETTINGS_SE_VOL, idx - 22);
                    this->read_state(SETTINGS_SE_VOL, 23);
                    break;
                case 33:
                case 34:
                case 35:
                case 36:
                case 37:
                case 38:
                case 39:
                case 40:
                case 41:
                case 42:
                    global.settings->set_int(SETTINGS_VO_VOL, idx - 32);
                    this->read_state(SETTINGS_VO_VOL, 33);
                    break;
            }
            break;
    }
    printf("Menu::next_state(%i) last state %i new state %i\n", idx, state_last, this->m_state);
}

int Menu::get_state()
{
    return this->m_state;
}

regions_t *Menu::get_regions()
{
    return &this->m_regions;
}

region_t *Menu::get_dlg_regions()
{
    return this->m_dlg_regions;
}

float parse_value(const char **raw)
{
    float ret = atof(*raw);
    *raw = strchr(*raw, ' ') + 1;
    return ret;
}

void Menu::load_glmap(const char *name, bool is_menu)
{
    Stream *str = new Stream(name, FILE_OPEN_READ_ST);
    if (str->getFileStreamHandle() == INVALID_HANDLE_VALUE)
    {
        ERROR_MESSAGE("Menu: Unable to open GLMAP %s\n", name);
        return;
    }
    Parser *map = new Parser(str);
    int count = atoi(map->get_value("Regions"));
    for (int i=0 ; i<count ; i++)
    {
        region_t *region = new region_t;
        const char *raw = map->get_value(i + 1);
        region->state = atoi(raw);
        raw = strchr(raw, ' ') + 1;
        region->idx = i;
        region->x1 = parse_value(&raw);
        region->y1 = parse_value(&raw);
        region->x2 = parse_value(&raw);
        region->y2 = parse_value(&raw);
        region->chip = NULL;
        if (is_menu)
        {
            if (this->m_dlg_regions[0].state == 0)
                this->m_dlg_regions[0] = *region;
            else
                this->m_dlg_regions[1] = *region;
        }
        else
            this->m_regions.push_back(region);
        //NORMA_MESSAGE("Menu: Region %i %f:%f %f:%f\n", region->state, region->x1, region->y1, region->x2, region->y2);
    }
    delete map;
    delete str;
}

void Menu::load_chip(const char *name, bool is_menu)
{
    Stream *str = new Stream(name, FILE_OPEN_READ_ST);
    if (str->getFileStreamHandle() == INVALID_HANDLE_VALUE)
    {
        ERROR_MESSAGE("Menu: Unable to open CHIP_GLMAP %s\n", name);
        return;
    }
    Parser *map = new Parser(str);
    int count = atoi(map->get_value("Regions"));
    for (int i=0 ; i<count ; i++)
    {
        region_chip_t *region = new region_chip_t;
        const char *raw = map->get_value(i + 1);
        region->region = atoi(raw);
        raw = strchr(raw, ' ') + 1;
        region->state = atoi(raw);
        raw = strchr(raw, ' ') + 1;
        region->x1 = parse_value(&raw);
        region->y1 = parse_value(&raw);
        region->x2 = parse_value(&raw);
        region->y2 = parse_value(&raw);
        if (is_menu)
        {
            if (this->m_dlg_chips[0].state == 0)
                this->m_dlg_chips[0] = *region;
            else
                this->m_dlg_chips[1] = *region;
        }
        else
            this->m_chips.push_back(region);
        NORMA_MESSAGE("Menu: Chip region %i %f:%f %f:%f\n", region->region, region->x1, region->y1, region->x2, region->y2);
    }
    delete map;
    delete str;
}

void Menu::load_menu(const char *name)
{
    this->unload_menu();

    char base_text[MAX_PATH];
    strcpy(base_text, MENU_PATH);
    strcat(base_text, name);

    char chip_text[MAX_PATH];
    strcpy(chip_text, base_text);
    strcat(chip_text, "_chip");

    char map_path[MAX_PATH];
    strcpy(map_path, chip_text);
    strcat(map_path, ".glmap");
    load_chip(map_path);

    strcpy(map_path, base_text);
    strcat(map_path, ".glmap");
    load_glmap(map_path);


    strcpy(map_path, chip_text);
    strcat(map_path, ".png");
    global._interface->load_tex(base_text, LAYER_MENU);
    global._interface->load_tex(map_path, LAYER_MENU_OVERLAY);
}

void Menu::unload_menu()
{
    regions_t::iterator end = this->m_regions.end();
    for (regions_t::iterator it=this->m_regions.begin() ; it<end ; it++)
    {
        delete *it;
    }
    this->m_regions.clear();

    regions_chip_t::iterator chip_end = this->m_chips.end();
    for (regions_chip_t::iterator it=this->m_chips.begin() ; it<chip_end ; it++)
    {
        delete *it;
    }
    this->m_chips.clear();
    global._interface->unload_tex(LAYER_MENU);
    global._interface->unload_tex(LAYER_MENU_OVERLAY);
}

void Menu::menu_exit_effect()
{
    global.engine->stop();
    //global.engine->add_event(EVENT_WHITE_FADE, "IN", "00:00:00", "00:01:00", true);
    //global.engine->add_event(EVENT_WHITE_FADE, "OUT", "00:01:00", "00:02:00", true);
    global.engine->add_event(EVENT_WHITE_FADE, "IN", "00:00:00", "00:00:30", true);
    global.engine->add_event(EVENT_WHITE_FADE, "OUT", "00:00:30", "00:01:00", true);
    global.engine->start();
    //global.engine->dump();
}

void Menu::menu_start_effect()
{
    //global.engine->stop();
    //global.engine->start();
}

void Menu::load_dlg(const char *name)
{
    char base_text[MAX_PATH];
    strcpy(base_text, MENU_PATH);
    strcat(base_text, name);

    char chip_text[MAX_PATH];
    strcpy(chip_text, base_text);
    strcat(chip_text, "_chip");

    char map_path[MAX_PATH];
    strcpy(map_path, chip_text);
    strcat(map_path, ".glmap");
    load_chip(map_path, true);

    strcpy(map_path, base_text);
    strcat(map_path, ".glmap");
    load_glmap(map_path, true);


    strcpy(map_path, chip_text);
    strcat(map_path, ".png");
    global._interface->load_tex(base_text, LAYER_DLG);
    global._interface->load_tex(map_path, LAYER_DLG_OVERLAY);
    this->m_dlg_active = true;
}

void Menu::unload_dlg()
{
    global._interface->unload_tex(LAYER_DLG);
    global._interface->unload_tex(LAYER_DLG_OVERLAY);
    this->m_dlg_active = false;
    memset(this->m_dlg_regions, 0, sizeof(region_t) * 2);
    memset(this->m_dlg_chips, 0, sizeof(region_chip_t) * 2);
}

void Menu::show_splash()
{
    global.engine->stop();
   /* global.engine->add_event(EVENT_BLACK_FADE, "OUT", "00:00:00", "00:02:00", false);
    global._interface->load_tex(global.settings->get_string("LogoFile"), LAYER_MENU);
    global.engine->add_event(EVENT_BLACK_FADE, "IN", "00:04:00", "00:05:00", true);*/
    global.engine->add_event(EVENT_BLACK_FADE, "OUT", "00:00:00", "00:00:30", false);
    global._interface->load_tex(global.settings->get_string("LogoFile"), LAYER_MENU);
    global.engine->add_event(EVENT_BLACK_FADE, "IN", "00:00:50", "00:01:00", true);
    global.engine->start();
}

void Menu::show_title()
{
    global.sound->play_system_bgm(SND_TITLE);
    this->load_menu("Title/Title");
}

void Menu::show_load()
{
    global.sound->play_system_bgm(SND_SYSTEM);
    this->load_menu("Saveload/Load");
}

void Menu::read_state(const char *name, int idx_0, int idx_1)
{
    if (global.settings->get_bool(name))
        this->m_regions.at(idx_1)->state = MENU_SELECTED;
    else
        this->m_regions.at(idx_0)->state = MENU_SELECTED;
}

void Menu::read_state(const char *name, int idx_start)
{
    int val = global.settings->get_int(name);

    this->m_regions.at(idx_start + 9)->state = MENU_DEFAULT;
    this->m_regions.at(idx_start + 8)->state = MENU_DEFAULT;
    this->m_regions.at(idx_start + 7)->state = MENU_DEFAULT;
    this->m_regions.at(idx_start + 6)->state = MENU_DEFAULT;
    this->m_regions.at(idx_start + 5)->state = MENU_DEFAULT;
    this->m_regions.at(idx_start + 4)->state = MENU_DEFAULT;
    this->m_regions.at(idx_start + 3)->state = MENU_DEFAULT;
    this->m_regions.at(idx_start + 2)->state = MENU_DEFAULT;
    this->m_regions.at(idx_start + 1)->state = MENU_DEFAULT;
    this->m_regions.at(idx_start + 0)->state = MENU_DEFAULT;

    switch (val)
    {
        case 10:this->m_regions.at(idx_start + 9)->state = MENU_SELECTED;
        case 9: this->m_regions.at(idx_start + 8)->state = MENU_SELECTED;
        case 8: this->m_regions.at(idx_start + 7)->state = MENU_SELECTED;
        case 7: this->m_regions.at(idx_start + 6)->state = MENU_SELECTED;
        case 6: this->m_regions.at(idx_start + 5)->state = MENU_SELECTED;
        case 5: this->m_regions.at(idx_start + 4)->state = MENU_SELECTED;
        case 4: this->m_regions.at(idx_start + 3)->state = MENU_SELECTED;
        case 3: this->m_regions.at(idx_start + 2)->state = MENU_SELECTED;
        case 2: this->m_regions.at(idx_start + 1)->state = MENU_SELECTED;
        case 1: this->m_regions.at(idx_start + 0)->state = MENU_SELECTED;
    }
}

void Menu::show_settings()
{
    global.sound->play_system_bgm(SND_SYSTEM);
    this->load_menu("Option/Option_def");
    this->read_state("DisplayType", 4, 3);
    this->read_state("WindowMode", 5, 6);
    this->read_state("SuperSkip", 8, 7);
    this->read_state("Skip", 10, 9);
    this->read_state("TextView", 12, 11);
}

void Menu::show_settings_snd()
{
    this->load_menu("Option/Option_sound");
    this->read_state(SETTINGS_MEN_VO, 10, 9);
    this->read_state(SETTINGS_MUTE, 12, 11);
    this->read_state(SETTINGS_BGM_VOL, 13);
    this->read_state(SETTINGS_SE_VOL, 23);
    this->read_state(SETTINGS_VO_VOL, 33);
}

void Menu::show_exit_dlg()
{
    if (this->m_in_game)
        this->load_dlg("Exit/Popup_title");
    else
        this->load_dlg("Exit/Popup_exit");
}

bool Menu::in_dlg()
{
    return this->m_dlg_active;
}
