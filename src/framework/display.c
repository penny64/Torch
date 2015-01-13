#include "display.h"
#include "libtcod.h"


void createDisplay() {
	TCOD_console_init_root(WINDOW_WIDTH, WINDOW_HEIGHT, "cGame - flagsdev 2015", false, TCOD_RENDERER_OPENGL);
	TCOD_sys_set_fps(FPS);
}

void displayLogic() {
	TCOD_console_flush();
}
