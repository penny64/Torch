#include "libtcod.h"
#include "framework/display.h"
#include "items.h"


TCOD_console_t ITEM_CONSOLE;


void itemSetup() {
	ITEM_CONSOLE = TCOD_console_new(WINDOW_WIDTH, WINDOW_HEIGHT);

	TCOD_console_set_default_background(ITEM_CONSOLE, TCOD_color_RGB(255, 0, 255));
	TCOD_console_set_key_color(ITEM_CONSOLE, TCOD_color_RGB(255, 0, 255));
}

TCOD_console_t getItemConsole() {
	return ITEM_CONSOLE;
}