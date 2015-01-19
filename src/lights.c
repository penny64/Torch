#include <stdlib.h>

#include "libtcod.h"
#include "framework/display.h"
#include "framework/numbers.h"
#include "framework/draw.h"
#include "level.h"
#include "lights.h"


static light *LIGHTS = NULL;
int (*lightMap)[255];
TCOD_map_t LIGHT_MAP;


light *createLight(int x, int y) {
	light *_c, *_p_c;
	
	_c = malloc(sizeof(light));
	_c->x = x;
	_c->y = y;
	//_c->vx = 0;
	//_c->vy = 0;
	_c->prev = NULL;
	_c->next = NULL;
	_c->fov = copyLevelMap();
	
	TCOD_map_compute_fov(_c->fov, x, y, 32, 1, FOV_SHADOW);
	
	if (LIGHTS == NULL) {
		LIGHTS = _c;
	} else {
		_p_c = LIGHTS;
		
		LIGHTS->next = _c;
		_c->prev = _p_c;
	}

	return _c;
}

light *getLights() {
	return LIGHTS;
}

TCOD_map_t getLightMap() {
	return LIGHT_MAP;
}

void startLights() {
	LIGHT_MAP = TCOD_map_new(WINDOW_WIDTH, WINDOW_HEIGHT);
}

void _drawLight(light *lght) {
	int x, y;
	int distMod;
	//TCOD_console_t lightConsole = getLightConsole();
	
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
	
	//TCOD_console_clear(lightConsole);
	
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
				
				
				if (highest > 15) {
					highest -= 15;
					
					if (highest <= 0) {
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
				drawCharBackEx(lightConsole, x, y, TCOD_color_RGB(65, 65, 25), TCOD_BKGND_ALPHA(distMod / 64.f));
				TCOD_map_set_properties(LIGHT_MAP, x, y, 1, 1);
			}
		}
	}
}
