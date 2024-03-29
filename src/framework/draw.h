#include "libtcod.h"


void setChar(TCOD_console_t, int, int, int);
void setCharEx(TCOD_console_t, int, int, int, TCOD_color_t);
void drawChar(TCOD_console_t, int, int, int, TCOD_color_t, TCOD_color_t);
void drawCharEx(TCOD_console_t, int, int, int, TCOD_color_t, TCOD_color_t, TCOD_bkgnd_flag_t);
void drawCharBack(TCOD_console_t, int, int, TCOD_color_t);
void drawCharBackEx(TCOD_console_t, int, int, TCOD_color_t, TCOD_bkgnd_flag_t);
void drawString(TCOD_console_t, int, int, TCOD_color_t, TCOD_color_t, const char*, ...);
