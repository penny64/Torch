#include "display.h"
#include "libtcod.h"


void createDisplay() {
	TCOD_console_set_custom_font("data/terminal.png",  TCOD_FONT_LAYOUT_ASCII_INCOL, 0, 0);
	TCOD_console_init_root(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, false, TCOD_RENDERER_OPENGL);
	TCOD_sys_set_fps(FPS);

	LEVEL_CONSOLE = TCOD_console_new(WINDOW_WIDTH, WINDOW_HEIGHT);
	ACTOR_CONSOLE = TCOD_console_new(WINDOW_WIDTH, WINDOW_HEIGHT);
}

void displayLogic() {
	TCOD_console_flush();
}
