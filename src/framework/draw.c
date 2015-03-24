#include "display.h"
#include "logging.h"
#include "draw.h"
#include "libtcod.h"


void setChar(TCOD_console_t console, int x, int y, int chr) {
	TCOD_console_set_char(console, x, y, chr);
}

void setCharEx(TCOD_console_t console, int x, int y, int chr, TCOD_color_t col) {
	TCOD_console_set_char(console, x, y, chr);
	TCOD_console_set_char_foreground(console, x, y, col);
}

void drawChar(TCOD_console_t console, int x, int y, int chr, TCOD_color_t foreColor, TCOD_color_t backColor) {
	TCOD_console_put_char_ex(console, x, y, chr, foreColor, backColor);
}

void drawCharBack(TCOD_console_t console, int x, int y, TCOD_color_t color) {
	TCOD_console_set_char_background(console, x, y, color, TCOD_BKGND_SET);
}

void drawCharBackEx(TCOD_console_t console, int x, int y, TCOD_color_t color, TCOD_bkgnd_flag_t flag) {
	TCOD_console_set_char_background(console, x, y, color, flag);
}

void drawString(TCOD_console_t console, int x, int y, const char *string) {
	//TCOD_console_set_background_flag(console, TCOD_BKGND_SET);
	//TCOD_console_set_default_background(console, TCOD_color_RGB(1, 1, 1));
	//TCOD_console_set_default_foreground(console, TCOD_color_RGB(255, 245, 245));
	TCOD_console_print_ex(console, x, y, TCOD_BKGND_SET, TCOD_LEFT, string, TCOD_COLCTRL_1, TCOD_COLCTRL_STOP);
	//TCOD_console_print(console, x, y, string);
	//TCOD_console_set_default_background(console, TCOD_color_RGB(255, 0, 255));
}
