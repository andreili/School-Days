#include "defines.h"
#include "interface.h"
#include "game.h"
#include "fs.h"
#include "stream.h"
#include "menu.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <png.h>

#define USE_OPENGL

#define SCR_BITS 32

Interface::Interface()
{
#ifdef USE_OPENGL
#ifdef _WIN32
#elif defined __unix__
    this->m_att[0] = GLX_RGBA;
    this->m_att[1] = GLX_DEPTH_SIZE;
    this->m_att[2] = 24;
    this->m_att[3] = GLX_DOUBLEBUFFER;
    this->m_att[4] = None;
    this->m_fade = 0;
#else
    #error here!
#endif
#else
#endif
}

Interface::~Interface()
{
#ifdef SD_DEBUG
    printf("Interface: free\n");
#endif
    unload_tex(TEXTURE_BLACK_IDX);
    unload_tex(TEXTURE_WHITE_IDX);
    kill_wnd();
}

#ifdef _WIN32
#include "interface_win"
#elif defined __unix__
#include "interface_unix"
#else
    #error here!
#endif

void Interface::init_gl()
{
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    glEnable(GL_TEXTURE_2D);

    glEnable(GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#ifdef USE_OPENGL
    glDisable(GL_COLOR_MATERIAL);
#endif

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
#ifdef USE_OPENGL
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
#endif

    for (int i=0 ; i<20 ; i++)
        this->m_textures[i] = uint32_t(-1);
}

void Interface::resize_gl(GLsizei width, GLsizei height)
{
    if (height==0)										// Prevent A Divide By Zero By
    {
        height=1;										// Making Height Equal One
    }

    this->m_width = width;
    this->m_height = height;

    glViewport(0,0,width,height);						// Reset The Current Viewport

    glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
    glLoadIdentity();									// Reset The Projection Matrix

    // Calculate The Aspect Ratio Of The Window
    glOrtho(0., 1., 1., 0., 1., 30.);

    glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
    glLoadIdentity();									// Reset The Modelview Matrix
}

bool Interface::init(const char *title, int w, int h, bool fullscreen)
{
#ifdef USE_OPENGL
#else
    static const EGLint attribs[] =
    {
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_NONE
    };
    static const EGLint ctx_attribs[] =
    {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };
#endif

    this->m_fullscreen = fullscreen;

    if (!this->create_wnd(title, w, h))
    {
        ERROR_MESSAGE("Interface: Cannot create window\n");
        return false;
    }

    this->resize_gl(w, h);	        				// Set Up Our Perspective GL Screen

    this->init_gl();					    		// Initialize Our Newly Created GL Window

    if (!load_tex(TEXTURE_BLACK, TEXTURE_BLACK_IDX) || !load_tex(TEXTURE_WHITE, TEXTURE_WHITE_IDX))
    {
        ERROR_MESSAGE("Interface: Unable to load system images\n");
        return false;
    }

    return true;
}

#ifndef USE_OPENGL
static GLint attr_pos = 0, attr_color = 1;
#endif

void draw_quad(GLuint texture, int z)
{
    if (texture == uint32_t(-1))
        return;
    glBindTexture(GL_TEXTURE_2D, texture);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3i(0, 0,  z);
    glTexCoord2f(1.0f, 0.0f); glVertex3i(1, 0,  z);
    glTexCoord2f(1.0f, 1.0f); glVertex3i(1, 1,  z);
    glTexCoord2f(0.0f, 1.0f); glVertex3i(0, 1,  z);
    glEnd();
}

void draw_circle(float cx, float cy, float r, int num_segments)
{
	float theta = 2 * 3.1415926 / float(num_segments);
	float c = cosf(theta);//precalculate the sine and cosine
	float s = sinf(theta);
	float t;

	float x = r;//we start at angle = 0
	float y = 0;

	glBegin(GL_LINE_LOOP);
	for(int ii = 0; ii < num_segments; ii++)
	{
		glVertex2f(x + cx, y + cy);//output vertex

		//apply the rotation matrix
		t = x;
		x = c * x - s * y;
		y = s * t + c * y;
	}
	glEnd();
}

void Interface::draw()
{
    //printf("draw\n");
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

#ifdef USE_OPENGL
    /*glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1., 1., -1., 1., 1., 30.);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();*/
    gluLookAt(0., 0., 21., 0., 0., 0., 0., 1., 0.);

    glColor3f(1., 1., 1.);
    draw_quad(this->m_textures[LAYER_BG], LAYER_BG);
    //draw_quad(this->m_textures[LAYER_BG_OVERLAY_0], LAYER_BG_OVERLAY_0);
    //draw_quad(this->m_textures[LAYER_BG_OVERLAY_1], LAYER_BG_OVERLAY_1);
    //draw_quad(this->m_textures[LAYER_BG_OVERLAY_2], LAYER_BG_OVERLAY_2);

    draw_quad(this->m_textures[LAYER_TITLE_BASE], LAYER_TITLE_BASE);

    draw_quad(this->m_textures[LAYER_MENU], LAYER_MENU);
    if (!global.menu->in_dlg())
    {
        regions_t regions = *global.menu->get_regions();
        int count = regions.size();
        for (int i=0 ; i<count ; i++)
        {
            region_t *region = regions.at(i);
            region_chip_t *chip = region->chip;
            if ((region->state != MENU_DEFAULT) && (chip))
            {
                glBindTexture(GL_TEXTURE_2D, this->m_textures[LAYER_MENU_OVERLAY]);
                glBegin(GL_QUADS);
                    glTexCoord2f(chip->x1, chip->y1); glVertex3f(region->x1, region->y1, LAYER_MENU_OVERLAY);
                    glTexCoord2f(chip->x2, chip->y1); glVertex3f(region->x2, region->y1, LAYER_MENU_OVERLAY);
                    glTexCoord2f(chip->x2, chip->y2); glVertex3f(region->x2, region->y2, LAYER_MENU_OVERLAY);
                    glTexCoord2f(chip->x1, chip->y2); glVertex3f(region->x1, region->y2, LAYER_MENU_OVERLAY);
                glEnd();
            }
        }
    }

    //draw_quad(this->m_textures[LAYER_SYS_BASE], LAYER_SYS_BASE);
    if (global.menu->in_dlg())
    {
        draw_quad(this->m_textures[LAYER_DLG], LAYER_DLG);
        region_t *dlg_regions = global.menu->get_dlg_regions();
        for (int i=0 ; i<2 ; i++)
        {
            region_t *region = &dlg_regions[i];
            region_chip_t *chip = region->chip;
            if ((region->state != MENU_DEFAULT) && (chip))
            {
                glBindTexture(GL_TEXTURE_2D, this->m_textures[LAYER_DLG_OVERLAY]);
                glBegin(GL_QUADS);
                    glTexCoord2f(chip->x1, chip->y1); glVertex3f(region->x1, region->y1, LAYER_DLG_OVERLAY);
                    glTexCoord2f(chip->x2, chip->y1); glVertex3f(region->x2, region->y1, LAYER_DLG_OVERLAY);
                    glTexCoord2f(chip->x2, chip->y2); glVertex3f(region->x2, region->y2, LAYER_DLG_OVERLAY);
                    glTexCoord2f(chip->x1, chip->y2); glVertex3f(region->x1, region->y2, LAYER_DLG_OVERLAY);
                glEnd();
            }
        }
    }
    //for (int i=0 ; i<LAYERS_COUNT ; i++)
        //draw_quad(this->m_textures[i], i);

    if (this->m_fade_color != -1)
    {
        glColor4d(this->m_fade_color, this->m_fade_color, this->m_fade_color, this->m_fade);
        if (this->m_fade_color)
            draw_quad(this->m_texture_white, LAYER_OVERLAY);
        else
            draw_quad(this->m_texture_black, LAYER_OVERLAY);
    }

#ifdef _WIN32
    SwapBuffers(this->m_dc);
#elif defined __unix__
    glXSwapBuffers(this->m_dpy, this->m_win);
#else
    #error here!
#endif

#else

    glViewport(0, 0, this->m_gwa.width, this->m_gwa.height);
    for (int i=0 ; i<=LAYERS_COUNT ; i++)
        if (this->m_textures[i] != uint32_t(-1))
        {
            glBindTexture(GL_TEXTURE_2D, this->m_textures[i]);

            GLint box[] =
            {
                -1, -1, i,
                1, -1, i,
                1,  1, i,
                -1,  1, i
            };
            GLfloat tex[] =
            {
                0.0, 1.0,
                1.0, 1.0,
                1.0, 0.0,
                0.0, 0.0
            };

            glVertexAttribPointer(attr_pos, 3, GL_INT, GL_FALSE, 0, box);
            glVertexAttribPointer(attr_color, 2, GL_FLOAT, GL_FALSE, 0, tex);
            glEnableVertexAttribArray(attr_pos);
            glEnableVertexAttribArray(attr_color);

            //glVertexPointer(3, GL_FLOAT, 0, box);
            //glTexCoordPointer(2, GL_FLOAT, 0, tex);

            glDrawArrays(GL_TRIANGLES, 0, 3);

            glDisableVertexAttribArray(attr_pos);
            glDisableVertexAttribArray(attr_color);
        }

    eglSwapBuffers(this->m_dpy, this->m_surf);
#endif

}

bool Interface::load_tex(const char *name, int idx)
{
    char fn[MAX_PATH];
    strcpy(fn, name);
    if (!strstr(name, ".png"))
    {
        strcat(fn, ".png");
    }

    Stream *str = global.fs->open(fn);
    if ((str == NULL) || (str->getFileStreamHandle() == INVALID_HANDLE_VALUE))
    {
        ERROR_MESSAGE("Interface: Doesn\'t load texture %s\n", fn);
        return false;
    }
    uint32_t sz = str->getSize();
    char buf[sz];
    str->read(buf, sz);
    delete str;

    bool ret = this->load_tex(buf, sz, idx);
#ifdef SD_DEBUG
    if (ret)
        printf("Game: Texture %s loaded\n", fn);
    else
    {
        red_color();
        printf("Game: Doesn\'t load texture %s\n", fn);
        normal_color();
    }
#endif
    return ret;
}

bool Interface::load_tex(const char *buf, uint32_t sz, int idx)
{
    png_image image;
    memset(&image, 0, sizeof(png_image));
    image.version = PNG_IMAGE_VERSION;

    if (png_image_begin_read_from_memory(&image, buf, sz))
    {
        image.format = PNG_FORMAT_RGBA;

        png_bytep buffer = (png_bytep)malloc(PNG_IMAGE_SIZE(image));

        if (buffer != NULL)
        {
            if (png_image_finish_read(&image, NULL, buffer, 0, NULL))
            {
                if (idx == TEXTURE_BLACK_IDX)
                {
                    glGenTextures(1, &this->m_texture_black);
                    glBindTexture(GL_TEXTURE_2D, this->m_texture_black);
                }
                else if (idx == TEXTURE_WHITE_IDX)
                {
                    glGenTextures(1, &this->m_texture_white);
                    glBindTexture(GL_TEXTURE_2D, this->m_texture_white);
                }
                else
                {
                    glGenTextures(1, &this->m_textures[idx]);
                    glBindTexture(GL_TEXTURE_2D, this->m_textures[idx]);
                }
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Linear Filtering
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Linear Filtering
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width,
                             image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
            }
            free(buffer);
        }
    }
    else
    {
        ERROR_MESSAGE("Interface: Texture load error: %s\n", image.message);
        return false;
    }

    return true;
}

void Interface::unload_tex(int idx)
{
    if (idx == TEXTURE_BLACK_IDX)
    {
        glBindTexture(GL_TEXTURE_2D, this->m_texture_black);
        glDeleteTextures(1, &this->m_texture_black);
    }
    else if (idx == TEXTURE_WHITE_IDX)
    {
        glBindTexture(GL_TEXTURE_2D, this->m_texture_white);
        glDeleteTextures(1, &this->m_texture_white);
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, this->m_textures[idx]);
        glDeleteTextures(1, &this->m_textures[idx]);
        this->m_textures[idx] = uint32_t(-1);
    }
}

void Interface::set_fade(double value)
{
    this->m_fade = value;
}

void Interface::set_fade_color(int color)
{
    this->m_fade_color = color;
}
