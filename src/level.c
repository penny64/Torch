#include "framework/display.h"
#include "framework/draw.h"
#include "level.h"
#include "libtcod.h"


TCOD_console_t LEVEL_CONSOLE;


void levelSetup() {
	LEVEL_CONSOLE = TCOD_console_new(WINDOW_WIDTH, WINDOW_HEIGHT);
}

TCOD_console_t getLevelConsole() {
	return LEVEL_CONSOLE;
}

void generateLevel() {
	drawChar(LEVEL_CONSOLE, 10, 10, (int)'#', TCOD_color_RGB(210, 200, 200), TCOD_color_RGB(110, 100, 100));
}