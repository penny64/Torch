#include "libtcod.h"
#include "framework/display.h"
#include "framework/actors.h"
#include "level.h"


void composeScene() {
	TCOD_console_t levelConsole = getLevelConsole();
	TCOD_console_t actorConsole = getActorConsole();
	
	TCOD_console_blit(levelConsole, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, 0, 0, 1, 1);
	TCOD_console_blit(actorConsole, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, 0, 0, 1, 0);
}