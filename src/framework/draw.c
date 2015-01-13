#include "libtcod.h"


void drawChar(TCOD_console_t console, int x, int y, int chr, TCOD_color_t foreColor, TCOD_color_t backColor) {
	TCOD_console_put_char_ex(console, x, y, chr, foreColor, backColor);
}