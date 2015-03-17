#include <stdlib.h>
#include <stdio.h>

#include "libtcod.h"
#include "framework/display.h"
#include "framework/numbers.h"
#include "framework/actors.h"
#include "framework/draw.h"
#include "player.h"
#include "level.h"
#include "lights.h"
#include "items.h"


TCOD_console_t DYNAMIC_LIGHT_CONSOLE;
static light *LIGHTS = NULL;
static light *DYNAMIC_LIGHTS = NULL;
int (*lightMap)[255];
TCOD_map_t LIGHT_MAP;


light *createLight(int x, int y) {
	light *_c, *_p_c;
	
	_c = calloc(1, sizeof(light));
	_c->x = x;
	_c->y = y;
	_c->prev = NULL;
	_c->next = NULL;
	_c->fov = copyLevelMap();
	_c->lightMap = TCOD_map_new(WINDOW_WIDTH, WINDOW_HEIGHT);
	
	TCOD_map_compute_fov(_c->fov, x, y, 32, 1, FOV_SHADOW);
	
	if (LIGHTS == NULL) {
		LIGHTS = _c;
	} else {
		_p_c = LIGHTS;
		
		while (_p_c->next) {
			_p_c = _p_c->next;
		}
		
		_p_c->next = _c;
		_c->prev = _p_c;
	}

	return _c;
}

light *createDynamicLight(int x, int y, character *actor) {
	light *_c, *_p_c;
	
	_c = calloc(1, sizeof(light));
	_c->x = x;
	_c->y = y;
	_c->r_tint = 55;
	_c->g_tint = 50;
	_c->b_tint = 35;
	_c->fuelMax = 70;
	_c->fuel = _c->fuelMax;
	_c->noTint = 0;
	_c->owner = actor;
	_c->size = 8;
	_c->sizeMod = 1.f;
	_c->prev = NULL;
	_c->next = NULL;
	_c->fov = copyLevelMap();
	
	if (DYNAMIC_LIGHTS == NULL) {
		DYNAMIC_LIGHTS = _c;
	} else {
		_p_c = DYNAMIC_LIGHTS;

		while (_p_c->next) {
			_p_c = _p_c->next;
		}
		
		_p_c->next = _c;
		_c->prev = _p_c;
	}

	return _c;
}

void startLights() {
	LIGHT_MAP = TCOD_map_new(WINDOW_WIDTH, WINDOW_HEIGHT);
	DYNAMIC_LIGHT_CONSOLE = TCOD_console_new(WINDOW_WIDTH, WINDOW_HEIGHT);

	printf("Starting up lights...\n");

	TCOD_console_set_default_background(DYNAMIC_LIGHT_CONSOLE, TCOD_color_RGB(0, 0, 0));
	TCOD_console_set_key_color(DYNAMIC_LIGHT_CONSOLE, TCOD_color_RGB(255, 0, 255));
	TCOD_console_clear(DYNAMIC_LIGHT_CONSOLE);
}

void lightsShutdown() {
	light *next, *ptr = DYNAMIC_LIGHTS;
	
	printf("Cleaning up lights...\n");
	
	while (ptr != NULL) {
		next = ptr->next;
		
		TCOD_map_delete(ptr->fov);
		
		if (ptr->lightMap) {
			TCOD_map_delete(ptr->lightMap);
		}
		
		free(ptr);
		
		ptr = next;
	}
}

void deleteDynamicLight(light *lght) {
	if (lght == DYNAMIC_LIGHTS) {
		DYNAMIC_LIGHTS = lght->next;
	} else {
		lght->prev->next = lght->next;

		if (lght->next) {
			lght->next->prev = lght->prev;
		}
	}

	free(lght);
}

light *getLights() {
	return LIGHTS;
}

light *getDynamicLights() {
	return DYNAMIC_LIGHTS;
}

TCOD_console_t getDynamicLightConsole() {
	return DYNAMIC_LIGHT_CONSOLE;
}

TCOD_map_t getLightMap() {
	return LIGHT_MAP;
}

void _initDynamicLight(light *lght) {
	if (lght->fov) {
		TCOD_map_delete(lght->fov);
	} else {
		printf("Trying to init a light with no existing fov.\n");
	}
	
	if (lght->lightMap) {
		TCOD_map_delete(lght->lightMap);
	} else {
		printf("Trying to init a light with no existing light map.\n");
	}
	
	lght->fov = copyLevelMap();
	lght->lightMap = copyLevelMap();
	
	TCOD_map_compute_fov(lght->fov, lght->x, lght->y, lght->size, 1, FOV_SHADOW);
}

void resetLight(light *lght) {
	lght->fov = copyLevelMap();
	lght->lightMap = copyLevelMap();
	
	TCOD_map_compute_fov(lght->fov, lght->x, lght->y, lght->size, 1, FOV_SHADOW);
}

void refreshAllLights() {
	light *ptr = DYNAMIC_LIGHTS;
	
	while (ptr != NULL) {
		 _initDynamicLight(ptr);
		
		ptr = ptr->next;
	}
}

void _drawLight(light *lght) {
	int x, y;
	int distMod;
	TCOD_console_t itemConsole = getItemConsole();

	drawChar(itemConsole, lght->x, lght->y, (int)'i', TCOD_color_RGB(255, 255, 155), TCOD_color_RGB(0, 0, 0));
	
	for (y = lght->y - 64; y < lght->y + 64; y++) {
		for (x = lght->x - 64; x < lght->x + 64; x++) {
			if (TCOD_map_is_in_fov(lght->fov, x, y)) {
				distMod = 64 - distanceFloat(lght->x, lght->y, x, y);
				
				lightMap[x][y] = (int)distMod;
			}
		}
	}
}

void drawLights() {
	int x, y, x1, y1, highest, distMod, changed = 1;
	light *ptr = LIGHTS;
	TCOD_console_t lightConsole = getLightConsole();
	lightMap = malloc(sizeof(double[255][255]));
	
	for (y = 0; y <= WINDOW_HEIGHT; y++) {
		for (x = 0; x <= WINDOW_WIDTH; x++) {
			lightMap[x][y] = 0;
		}
	}

	while (ptr != NULL) {
		_drawLight(ptr);

		ptr = ptr->next;
	}
	
	while (changed) {
		changed = 0;
		
		for (y = 0; y <= WINDOW_HEIGHT; y++) {
			for (x = 0; x <= WINDOW_WIDTH; x++) {
				if (!isPositionWalkable(x, y)) {
					continue;
				}
				
				highest = -1;
				
				if (lightMap[x][y]) {
					continue;
				}
				
				for (y1 = -1; y1 <= 1; y1++) {
					for (x1 = -1; x1 <= 1; x1++) {
						if (!isPositionWalkable(x + x1, y + y1)) {
							continue;
						}
						
						distMod = lightMap[x + x1][y + y1];
						
						if (lightMap[x + x1][y + y1] > highest) {
							highest = lightMap[x + x1][y + y1];
						}
						
						//drawCharBackEx(lightConsole, x + x1, y + y1, TCOD_color_RGB(250 - distMod, 200 - distMod, 0), TCOD_BKGND_ALPHA(1.f));
					}
				}
				
				
				if (highest > 14) {
					highest -= 14;
					
					if (highest < 0) {
						continue;
					}
					
					lightMap[x][y] = highest;
					changed = 1;
				}
			}
		}
	}
	
	for (y = 0; y <= WINDOW_HEIGHT; y++) {
		for (x = 0; x <= WINDOW_WIDTH; x++) {
			if (lightMap[x][y]) {
				distMod = lightMap[x][y];
				drawCharBackEx(lightConsole, x, y, TCOD_color_RGB(65, 65, 65), TCOD_BKGND_ALPHA(distMod / 64.f));
				TCOD_map_set_properties(LIGHT_MAP, x, y, 1, 1);
			}
			
		}
	}
}

void _lightLogic(light *lght) {
	int newSize = lght->size;
	
	if (lght->fuel < lght->size) {
		newSize -= lght->size - lght->fuel;
	}
	
	TCOD_map_compute_fov(lght->fov, lght->x, lght->y, newSize, 1, FOV_SHADOW);
	
	if (lght->fuel > 0) {
		lght->fuel --;
	}

	if (lght->fuel < 0) {
		printf("Warning: Light has less than 0 fuel.\n");

		lght->fuel = 0;
	}
}

void lightLogic() {
	light *ptr = DYNAMIC_LIGHTS;
	
	while (ptr != NULL) {
		_lightLogic(ptr);
		
		ptr = ptr->next;
	}
}

void _drawDynamicLight(light *lght) {
	int x, y, penalty, r_tint, g_tint, b_tint;
	float distMod, alpha;
	character *player = getPlayer();
	TCOD_map_t levelMap = getLevelMap();
	
	if (!lght->lightMap) {
		return;
	}
	
	TCOD_map_clear(lght->lightMap, 0, 0);
	
	if (!lght->fuel) {
		return;
	}
	
	for (y = lght->y - 32; y < lght->y + 32; y++) {
		for (x = lght->x - 32; x < lght->x + 32; x++) {
			if (x < 0 || x >= WINDOW_WIDTH || y < 0 || y >= WINDOW_HEIGHT) {
				continue;
			}
			
			if (TCOD_map_is_in_fov(lght->fov, x, y)) {
				if (lght->fuel < lght->size) {
					penalty = lght->size - lght->fuel;
				} else {
					penalty = 0;
				}
				
				distMod = lght->size - (distanceFloat(lght->x, lght->y, x, y) + penalty);
				
				if (distMod <= 0) {
					TCOD_map_set_properties(lght->lightMap, x, y, 0, 0);
				} else {
					TCOD_map_set_properties(lght->lightMap, x, y, 1, 1);
				}

				if (isPositionWalkable(x, y)) {
					distMod -= getRandomFloat(0, .5);
				}
				
				if (distMod < 0) {
					distMod = 0;
				} else if (distMod > lght->size / 2) {
					distMod = lght->size / 2;
				}
				
				alpha = (distMod / (float) lght->size);
				alpha *= lght->sizeMod;

				if (lght->noTint) {
					r_tint = 1;
					g_tint = 1;
					b_tint = 1;
				} else {
					if (!TCOD_map_is_walkable(levelMap, x, y)) {
						r_tint = 55 + RED_SHIFT;
						g_tint = 55;
						b_tint = 55;

						if (alpha > .45) {
							alpha = .45;
						}
					} else {
						r_tint = lght->r_tint + RED_SHIFT;
						g_tint = lght->g_tint;
						b_tint = lght->b_tint;

						if (alpha > .75) {
							alpha = .75;
						}
					}
				}
				
				if (!player || TCOD_map_is_in_fov(player->fov, x, y)) {
					drawCharBackEx(DYNAMIC_LIGHT_CONSOLE, x, y, TCOD_color_RGB(r_tint, g_tint, b_tint), TCOD_BKGND_ADDALPHA(alpha));
				}
			}
		}
	}
}

void drawDynamicLights() {
	light *ptr = DYNAMIC_LIGHTS;

	TCOD_console_clear(DYNAMIC_LIGHT_CONSOLE);
	
	while (ptr != NULL) {
		_drawDynamicLight(ptr);

		ptr = ptr->next;
	}
}

int isPositionLit(int x, int y) {
	light *lght = DYNAMIC_LIGHTS;
	
	while (lght != NULL) {
		if (lght->lightMap && TCOD_map_is_walkable(lght->lightMap, x, y)) {
			return 1;
		}
		
		lght = lght->next;
	}
	
	return 0;
}
