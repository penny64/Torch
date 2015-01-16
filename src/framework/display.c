#include "display.h"
#include "libtcod.h"


void createDisplay() {
	TCOD_console_set_custom_font("data/terminal_2x.png",  TCOD_FONT_LAYOUT_ASCII_INCOL, 0, 0);
	//TCOD_console_set_custom_font("data/terminal16x16_gs_ro.png",  TCOD_FONT_LAYOUT_ASCII_INROW|TCOD_FONT_TYPE_GREYSCALE, 0, 0);
	TCOD_console_init_root(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, false, TCOD_RENDERER_OPENGL);
	TCOD_sys_set_fps(FPS);
}

void displayLogic() {
	TCOD_console_flush();
}
