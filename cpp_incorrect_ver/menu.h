#ifndef _MENU_
#define _MENU_

#include "defines.h"
#include "stream.h"
#include "interface.h"
#include <vector>

using namespace std;

#define MENU_PATH "." PATH_DELIMITER_STR "System" PATH_DELIMITER_STR

#define MENU_INIT               0
#define MENU_SPLASH             1
#define MENU_TITLE              2
#define MENU_PRE_TITLE          102
#define MENU_EXIT_DLG           3
#define MENU_EXIT               4
#define MENU_SETTINGS           5
#define MENU_PRE_SETTINGS       105
#define MENU_SETTINGS_SOUND     6
#define MENU_LOAD               7
#define MENU_PRE_LOAD           107
#define MENU_REPLAY             8
#define MENU_PRE_REPLAY         108
#define MENU_ROUTE_MAP          9
#define MENU_NEW_GAME           10
#define MENU_GAME               11
#define MENU_SAVE               12
#define MENU_HISTORY            13
#define MENU_TITLE_DLG          14
#define MENU_CLOSE              15

#define MENU_DEFAULT        0
#define MENU_MOUSE_OVER     1
#define MENU_DISABLE        2
#define MENU_SELECTED       3
#define MENU_SELECTED_MOUSE 4

typedef struct
{
    uint32_t    region;
    uint32_t    state;
    float       x1;
    float       y1;
    float       x2;
    float       y2;
} region_chip_t;

typedef struct
{
    int         idx;
    float       x1;
    float       y1;
    float       x2;
    float       y2;
    uint32_t    state;
    region_chip_t *chip;
} region_t;

typedef vector<region_t*> regions_t;
typedef vector<region_chip_t*> regions_chip_t;

class Menu
{
private:
    int             m_state;
    int             m_next_state;
    bool            m_in_game;
    bool            m_dlg_active;
    regions_t       m_regions;
    regions_chip_t  m_chips;
    region_t        m_dlg_regions[2];
    region_chip_t   m_dlg_chips[2];

    void load_menu(const char *name);
    void load_glmap(const char *name, bool is_menu = false);
    void load_chip(const char *name, bool is_menu = false);
    void unload_menu();
    void menu_exit_effect();
    void menu_start_effect();

    void load_dlg(const char *name);
    void unload_dlg();

    void read_state(const char *name, int idx_0, int idx_1);
    void read_state(const char *name, int idx_start);

    void show_splash();
    void show_title();
    void show_load();
    void show_settings();
    void show_settings_snd();

    void show_exit_dlg();

    bool region_check(region_t *region, CURSOR *pointer, int idx);
public:
    Menu();
    ~Menu();

    bool proc(CURSOR *pointer);
    void prev_state(int idx);
    void next_state(int idx);
    int get_state();

    bool in_dlg();

    regions_t *get_regions();
    region_t *get_dlg_regions();
};

#endif
