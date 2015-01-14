#include "logging.h"
#include "draw.h"
#include "libtcod.h"


void drawChar(TCOD_console_t console, int x, int y, int chr, TCOD_color_t foreColor, TCOD_color_t backColor) {
	TCOD_console_put_char_ex(console, x, y, chr, foreColor, backColor);
}

void drawCharBackEx(TCOD_console_t console, int x, int y, TCOD_color_t color, TCOD_bkgnd_flag_t flag) {
	TCOD_console_set_char_background(console, x, y, color, flag);
}
