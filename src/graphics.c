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
	float distMod;
	TCOD_map_t map = getLevelMap();
	TCOD_console_t shadowConsole = getShadowConsole();
	character *player = getPlayer();
	character *actor;

	TCOD_console_clear(shadowConsole);
	
	for (y = 0; y < WINDOW_HEIGHT; y++) {
		for (x = 0; x < WINDOW_WIDTH; x++) {
			visible = 0;
			actor = getActors();
			
			while (actor != NULL) {
				if (TCOD_map_is_in_fov(actor->fov, x, y) && TCOD_map_is_in_fov(map, x, y)) {
					visible = 1;
				}

				actor = actor->next;
			}
			
			if (!visible) {
				distMod = distanceFloat(player->x, player->y, x, y);

				drawCharBackEx(shadowConsole, x, y, TCOD_color_RGB(0, 0, 0), TCOD_BKGND_ALPHA(1 - (distMod / 255.f)));
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
