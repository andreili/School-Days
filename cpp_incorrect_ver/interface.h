#ifndef _INTERFACE_H_
#define _INTERFACE_H_

#define LAYER_BG            0
#define LAYER_BG_OVERLAY_0  1
#define LAYER_BG_OVERLAY_1  2
#define LAYER_BG_OVERLAY_2  3
#define LAYER_TITLE_BASE    4
#define LAYER_MENU          5
#define LAYER_MENU_OVERLAY  6
#define LAYER_SYS_BASE      7
#define LAYER_DLG           8
#define LAYER_DLG_OVERLAY   9
#define LAYER_OVERLAY       10

#define LAYERS_COUNT        10

#define TEXTURE_BLACK "System/Screen/Black.png"
#define TEXTURE_BLACK_IDX -1
#define TEXTURE_WHITE "System/Screen/White.png"
#define TEXTURE_WHITE_IDX -1

#include "defines.h"
#include <inttypes.h>
#include "script_engine.h"
#ifdef _WIN32
    #include <windows.h>
#elif defined __unix__
    #include <X11/X.h>
    #include <X11/Xlib.h>
#else
    #error here!
#endif
#ifdef USE_OPENGL
    #include <GL/gl.h>
    #ifndef _WIN32
        #include <GL/glx.h>
    #endif
    #include <GL/glu.h>
#else
    #include <GLES3/gl3.h>
    #include <GLES3/gl3ext.h>
    #include <EGL/egl.h>

    -lSDL -lEGL -lGLESv2
#endif

typedef enum
{
    RENDER_MENU,
    RENDER_GAME
} ERenderLevel;

typedef struct
{
    float   x;
    float   y;
    bool left_presed;
    bool rigth_presed;
} CURSOR;

class Interface
{
private:
#ifdef _WIN32
    HWND                    m_wnd;
    HGLRC                   m_rc;
    HDC                     m_dc;
    HINSTANCE               m_instance;
#elif defined __unix__
    Display                 *m_dpy;
    Window                  m_root;
    XVisualInfo             *m_vi;
    Colormap                m_cmap;
    XSetWindowAttributes    m_swa;
    Window                  m_win;
#else
    #error here!
#endif
    int                     m_width;
    int                     m_height;
#ifdef USE_OPENGL
#ifndef _WIN32
    GLXContext              m_glc;
    GLint                   m_att[5];
#endif
#else
    EGLContext              m_glc;
    EGLSurface              m_surf;
    EGLDisplay              m_egl_dpy;
#endif

#ifdef _WIN32
#elif defined __unix__
    XWindowAttributes       m_gwa;
    XEvent                  m_xev;
#else
    #error here!
#endif

    bool                    m_fullscreen;
    GLuint                  m_textures[20];
    GLuint                  m_texture_black;
    GLuint                  m_texture_white;
    double                  m_fade;
    int                     m_fade_color;

    CURSOR                  m_pointer;

    bool create_wnd(const char *title, int w, int h);
    void init_gl();
public:
    Interface();
    ~Interface();

    bool init(const char *title, int w, int h, bool fullscreen);
    void kill_wnd();
    void wnd_proc();
    void resize_gl(GLsizei width, GLsizei height);

    bool load_tex(const char *name, int idx);
    bool load_tex(const char *buf, uint32_t sz, int idx);
    void unload_tex(int idx);

    void set_fade(double value);
    void set_fade_color(int color);

    void draw();
};

#endif
