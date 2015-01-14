#include <stdio.h>

#include "libtcod.h"
#include "framework/display.h"
#include "framework/actors.h"
#include "framework/draw.h"
#include "framework/numbers.h"
#include "player.h"
#include "level.h"


int x, y;

void generateFov() {
	TCOD_map_t map = getLevelMap();
	character *player = getPlayer();
	
	TCOD_map_compute_fov(map, player->x, player->y, 36, 1, FOV_SHADOW);
}

void applyFov() {
	int visible;
	TCOD_map_t map = getLevelMap();
	TCOD_console_t shadowConsole = getShadowConsole();
	TCOD_console_clear(shadowConsole);
	character *player = getPlayer();
	character *actor;
	
	for (y = 0; y < WINDOW_HEIGHT; y++) {
		for (x = 0; x < WINDOW_WIDTH; x++) {
			visible = 0;
			actor = getActors();
			
			//if (!TCOD_map_is_in_fov(map, x, y)) {
			//	visible = 0;
				//drawCharBackEx(shadowConsole, x, y, TCOD_color_RGB(0, 0, 0), TCOD_BKGND_SET);
			//} else {
				while (actor != NULL) {
					//if (actor == player) {
					//	continue;
					//}
					
					if (TCOD_map_is_in_fov(actor->fov, x, y) && TCOD_map_is_in_fov(map, x, y)) {
						visible = 1;
					}

					actor = actor->next;
				}
			//}
			
			if (!visible) {
				drawCharBackEx(shadowConsole, x, y, TCOD_color_RGB(0, 0, 0), TCOD_BKGND_SET);
			}
		}
	}
}

void composeScene() {
	generateFov();
	
	TCOD_console_t levelConsole = getLevelConsole();
	TCOD_console_t actorConsole = getActorConsole();
	TCOD_console_t shadowConsole = getShadowConsole();
	TCOD_console_t fogConsole = getFogConsole();
	
	applyFov();
	
	TCOD_console_blit(levelConsole, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, 0, 0, 1, 1);
	TCOD_console_blit(actorConsole, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, 0, 0, 1, 0);
	TCOD_console_blit(shadowConsole, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, 0, 0, 1, 0.99f);
	TCOD_console_blit(fogConsole, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, 0, 0, 1, 0.3f);
}
