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
	
	TCOD_map_compute_fov(map, player->x, player->y, 24, 1, FOV_SHADOW);
}

void applyFov() {
	int x, y;
	int visible, visibleToPlayer;
	float distMod, fadeValue;
	TCOD_map_t map = getLevelMap();
	TCOD_console_t actorConsole = getActorConsole();
	TCOD_console_t seenConsole = getSeenConsole();
	TCOD_console_t shadowConsole = getShadowConsole();
	character *player = getPlayer();
	character *actor;

	TCOD_console_clear(shadowConsole);
	
	for (y = 0; y < WINDOW_HEIGHT; y++) {
		for (x = 0; x < WINDOW_WIDTH; x++) {
			visible = 0;
			visibleToPlayer = 1;
			actor = getActors();
			
			while (actor != NULL) {
				if (TCOD_map_is_in_fov(map, x, y) && TCOD_map_is_in_fov(actor->fov, x, y)) {
					visible = 1;
					
					if (visibleToPlayer && !TCOD_map_is_in_fov(player->fov, x, y)) {
						visibleToPlayer = 0;
					}
				}

				actor = actor->next;
			}
			
			if (!visible) {
				distMod = distanceFloat(player->x, player->y, x, y);
				distMod -= getRandomInt(0, 3);
				
				if (distMod < 0) {
					distMod = 0;
				}
				
				fadeValue = 1.f - ((float) distMod / 32.f);
				
				setChar(actorConsole, x, y, (int)' ');
				
				if (fadeValue < .03126) {
					drawCharBackEx(shadowConsole, x, y, TCOD_color_RGB(1, 0, 0), TCOD_BKGND_SET);
				} else {
					drawCharBackEx(shadowConsole, x, y, TCOD_color_RGB(55, 15, 15), TCOD_BKGND_ALPHA(fadeValue));
				}
			} else {
				drawCharBack(seenConsole, x, y, TCOD_color_RGB(255, 0, 255));
			}
		}
	}
}

void composeScene() {
	generateFov();
	
	TCOD_console_t levelConsole = getLevelConsole();
	TCOD_console_t actorConsole = getActorConsole();
	TCOD_console_t seenConsole = getSeenConsole();
	TCOD_console_t shadowConsole = getShadowConsole();
	TCOD_console_t fogConsole = getFogConsole();
	
	applyFov();
	
	TCOD_console_blit(levelConsole, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, 0, 0, 1, 1);
	TCOD_console_blit(actorConsole, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, 0, 0, 1, 0);
	TCOD_console_blit(fogConsole, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, 0, 0, 1, 0.3f);
	TCOD_console_blit(seenConsole, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, 0, 0, 1, 1);
	TCOD_console_blit(shadowConsole, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, 0, 0, 1, 0.55f);
	
}
