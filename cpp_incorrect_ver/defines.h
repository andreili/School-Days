#ifndef _DEFINES_H_
#define _DEFINES_H_

/*  needs from FFmpeg  */
#ifdef __cplusplus
 #define __STDC_CONSTANT_MACROS
 #ifdef _STDINT_H
  #undef _STDINT_H
 #endif
 # include <stdint.h>
#endif

#ifdef _WIN32
#define PRId64 "%I64d"
#define PRsize "%i"
#elif defined __unix__
#define PRId64 "%li"
#define PRsize "%lu"
#define MAX_PATH 256

typedef struct
{
    float x;
    float y;
} POINTFLOAT;

#else
    #error here!
#endif

#define SD_DEBUG

#define USE_OPENGL

#ifdef SD_DEBUG
#include <stdio.h>
#ifdef _WIN32
#define red_color() fprintf(stderr, "\x1B[31m")
#define blu_color() fprintf(stderr, "\x1B[34m")
#elif defined __unix__
#define red_color() printf("\x1B[31m")
#define blu_color() printf("\x1B[34m")
#else
    #error here!
#endif
#define normal_color() printf("\x1B[0m")

#ifdef _WIN32
//#define ERROR_MESSAGE(fmt, ...) MessageBox(NULL, fmt, 0, 0);
#define ERROR_MESSAGEex(msg) puts(msg);
#define ERROR_MESSAGE(fmt, ...) printf(fmt, ##__VA_ARGS__);
#define NORMA_MESSAGE(fmt, ...) printf(fmt, ##__VA_ARGS__);
#elif defined __unix__
#define ERROR_MESSAGEex(msg) red_color(); puts(msg); normal_color();
#define ERROR_MESSAGE(fmt, ...) red_color(); printf(fmt, ##__VA_ARGS__); normal_color();
#define NORMA_MESSAGE(fmt, ...) blu_color(); printf(fmt, ##__VA_ARGS__); normal_color();
#else
    #error here!
#endif
#else
#define ERROR_MESSAGE(fmt, ...) {}
#endif

class Game;
class Interface;
class FS;
class Settings;
class ScriptEngine;
class Sound;
class Video;
class Menu;

typedef struct
{
    Game        *game;
    Interface   *_interface;
    FS          *fs;
    Settings    *settings;
    ScriptEngine*engine;
    Sound       *sound;
    Video       *video;
    Menu        *menu;
} global_t;

extern global_t global;

#endif
