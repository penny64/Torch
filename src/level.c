#include <stdlib.h>
#include <stdio.h>

#include "framework/display.h"
#include "framework/draw.h"
#include "level.h"
#include "lights.h"
#include "libtcod.h"
#include "items.h"


TCOD_console_t LEVEL_CONSOLE;
TCOD_console_t LIGHT_CONSOLE;
TCOD_console_t SHADOW_CONSOLE;
TCOD_console_t FOG_CONSOLE;
TCOD_console_t SEEN_CONSOLE;
TCOD_map_t LEVEL_MAP;
TCOD_noise_t FOG_NOISE;
TCOD_random_t RANDOM;
int (*ROOM_MAP)[255];
int (*DIJKSTRA_MAP)[255];
int ROOM_COUNT;


void levelSetup() {
	LEVEL_CONSOLE = TCOD_console_new(WINDOW_WIDTH, WINDOW_HEIGHT);
	LIGHT_CONSOLE = TCOD_console_new(WINDOW_WIDTH, WINDOW_HEIGHT);
	SHADOW_CONSOLE = TCOD_console_new(WINDOW_WIDTH, WINDOW_HEIGHT);
	FOG_CONSOLE = TCOD_console_new(WINDOW_WIDTH, WINDOW_HEIGHT);
	SEEN_CONSOLE = TCOD_console_new(WINDOW_WIDTH, WINDOW_HEIGHT);
	LIGHT_CONSOLE = TCOD_console_new(WINDOW_WIDTH, WINDOW_HEIGHT);
	LEVEL_MAP = TCOD_map_new(WINDOW_WIDTH, WINDOW_HEIGHT);
	FOG_NOISE = TCOD_noise_new(2, TCOD_NOISE_DEFAULT_HURST, TCOD_NOISE_DEFAULT_LACUNARITY, NULL);
	RANDOM = TCOD_random_get_instance();

	TCOD_console_set_default_background(LEVEL_CONSOLE, TCOD_color_RGB(40, 30, 30));
	TCOD_console_set_default_background(LIGHT_CONSOLE, TCOD_color_RGB(0, 0, 0));
	TCOD_console_set_default_background(SHADOW_CONSOLE, TCOD_color_RGB(0, 0, 0));
	TCOD_console_set_default_background(SEEN_CONSOLE, TCOD_color_RGB(0, 0, 0));
	TCOD_console_set_key_color(LIGHT_CONSOLE, TCOD_color_RGB(255, 0, 255));
	TCOD_console_set_key_color(SHADOW_CONSOLE, TCOD_color_RGB(0, 0, 0));
	TCOD_console_set_key_color(SEEN_CONSOLE, TCOD_color_RGB(255, 0, 255));

	TCOD_console_clear(LEVEL_CONSOLE);
	TCOD_console_clear(LIGHT_CONSOLE);
	TCOD_console_clear(SHADOW_CONSOLE);
	TCOD_console_clear(SEEN_CONSOLE);
	
	TCOD_noise_set_type(FOG_NOISE, TCOD_NOISE_PERLIN);
	
	startLights();
}

int getRandomInt(int min, int max) {
	return TCOD_random_get_int(RANDOM, min, max);
}

float getRandomFloat(float min, float max) {
	return TCOD_random_get_float(RANDOM, min, max);
}

TCOD_console_t getLevelConsole() {
	return LEVEL_CONSOLE;
}

TCOD_console_t getLightConsole() {
	return LIGHT_CONSOLE;
}

TCOD_console_t getShadowConsole() {
	return SHADOW_CONSOLE;
}

TCOD_console_t getFogConsole() {
	return FOG_CONSOLE;
}

TCOD_noise_t getFogNoise() {
	return FOG_NOISE;
}

TCOD_map_t getLevelMap() {
	return LEVEL_MAP;
}

TCOD_map_t getSeenConsole() {
	return SEEN_CONSOLE;
}

TCOD_map_t copyLevelMap() {
	TCOD_map_t newMap = TCOD_map_new(WINDOW_WIDTH, WINDOW_HEIGHT);
	TCOD_map_copy(LEVEL_MAP, newMap);
	
	return newMap;
}

int isPositionWalkable(int x, int y) {
	return TCOD_map_is_walkable(LEVEL_MAP, x, y);
}

void carve(int x, int y) {
	int x1, y1;
	int i, ii;
	
	for (i = 0; i < TCOD_random_get_int(RANDOM, 24, 32); i++) {
		for (ii = 0; ii < TCOD_random_get_int(RANDOM, 0, 1); ii++) {
			for (y1 = -1 - ii; y1 <= 1 + ii; y1++) {
				for (x1 = -1; x1 <= 1; x1++) {
					if (x + x1 <= 1 || x + x1 >= WINDOW_WIDTH - 2 || y + y1 <= 1 || y + y1 >= WINDOW_HEIGHT - 2) {
						continue;
					}
					
					TCOD_map_set_properties(LEVEL_MAP, x + x1, y + y1, 1, 1);
				}
			}
		}
		
		while (TCOD_map_is_walkable(LEVEL_MAP, x, y)) {
			if (TCOD_random_get_int(RANDOM, 0, 1)) {
				x += TCOD_random_get_int(RANDOM, -1, 1);
			} else {
				y += TCOD_random_get_int(RANDOM, -1, 1);
			}
		}
	}
}

void smooth() {
	int x, y, i, x1, y1, count;
	
	for (i = 0; i < 32; i++) {
		TCOD_map_t mapCopy = copyLevelMap();
		
		for (y = 0; y < WINDOW_HEIGHT; y++) {
			for (x = 0; x < WINDOW_WIDTH; x++) {
				if (TCOD_map_is_walkable(mapCopy, x, y)) {
					continue;
				}
				
				count = 0;
				
				for (y1 = -1; y1 <= 1; y1++) {
					for (x1 = -1; x1 <= 1; x1++) {
						if ((y1 == -1 && x1 == 1) || (y1 == -1 && x1 == -1) || (y1 == 1 && x1 == 1) || (y1 == 1 && x1 == -1)) {
							continue;
						}
						
						if (TCOD_map_is_walkable(mapCopy, x, y)) {
							count ++;
						}
					}
				}
				
				if (count == 4) {
					TCOD_map_set_properties(LEVEL_MAP, x, y, 1, 1);
				}
			}
		}
	}
}

void placeLights() {
	int x, y, x1, y1, count;
	TCOD_map_t mapCopy = copyLevelMap();
	
	for (y = 0; y < WINDOW_HEIGHT; y++) {
		for (x = 0; x < WINDOW_WIDTH; x++) {
			if (!TCOD_map_is_walkable(mapCopy, x, y)) {
				continue;
			}
			
			count = 0;
			
			for (y1 = -1; y1 <= 1; y1++) {
				for (x1 = -1; x1 <= 1; x1++) {
					if (!TCOD_map_is_walkable(mapCopy, x + x1, y + y1)) {
						count ++;
					}
				}
			}
			
			if (count == 1 && !TCOD_random_get_int(RANDOM, 0, 5)) {
				createLight(x, y);
			}
		}
	}
}

void findRooms() {
	int i, x, y, x1, y1, w_x, w_y, oLen, cLen, added = 1;
	int (*openList)[WINDOW_WIDTH * WINDOW_HEIGHT] = malloc(sizeof(double[WINDOW_WIDTH * WINDOW_HEIGHT][WINDOW_WIDTH * WINDOW_HEIGHT]));
	int (*CLOSED_MAP)[255] = malloc(sizeof(double[255][255]));
	ROOM_MAP = malloc(sizeof(double[255][255]));
	ROOM_COUNT = 0;

	for (y = 0; y <= WINDOW_HEIGHT; y++) {
		for (x = 0; x <= WINDOW_WIDTH; x++) {
			CLOSED_MAP[x][y] = 0;
			ROOM_MAP[x][y] = 0;
		}
	}
	
	while (added) {
		cLen = 0;
		oLen = 0;
		added = 0;

		//Find starting position
		for (y = 0; y < WINDOW_HEIGHT; y++) {
			for (x = 0; x < WINDOW_WIDTH; x++) {
				if (!TCOD_map_is_walkable(LEVEL_MAP, x, y) || ROOM_MAP[x][y]) {
					continue;
				}

				openList[0][0] = x;
				openList[0][1] = y;
				oLen ++;

				break;
			}

			if (oLen) {
				break;
			}
		}

		while (cLen < oLen) {
			w_x = openList[cLen][0];
			w_y = openList[cLen][1];

			cLen ++;

			for (y1 = -1; y1 <= 1; y1++) {
				for (x1 = -1; x1 <= 1; x1++) {
					x = w_x + x1;
					y = w_y + y1;

					if (!TCOD_map_is_walkable(LEVEL_MAP, x, y)) {
						continue;
					}

					if (x <= 1 || x >= WINDOW_WIDTH - 2 || y <= 1 || y >= WINDOW_HEIGHT - 2) {
						continue;
					}

					if (ROOM_MAP[x][y]) {
						continue;
					}
					
					if (CLOSED_MAP[x][y] > 0) {
						continue;
					}

					if (!(y1 == 0 && x1 == 0)) {
						openList[oLen][0] = x;
						openList[oLen][1] = y;
					}

					CLOSED_MAP[x][y] = 1;
					oLen ++;
					added ++;
				}
			}
		}

		if (added) {
			ROOM_COUNT ++;

			printf("Found new room: %i\n", ROOM_COUNT);
		}

		for (i = 0; i < oLen; i++) {
			x = openList[i][0];
			y = openList[i][1];
			
			ROOM_MAP[x][y] = ROOM_COUNT;
		}
	}
}

void placeTunnels() {
	int x, y, x1, y1, w_x, w_y, mapUpdates, currentValue, neighborValue, lowestValue;;
	DIJKSTRA_MAP = malloc(sizeof(double[255][255]));

	while (ROOM_COUNT) {
		printf("Generating tunnels for room %i\n", ROOM_COUNT);

		//Find our first room
		for (y = 2; y < WINDOW_HEIGHT - 1; y++) {
			for (x = 2; x < WINDOW_WIDTH - 1; x++) {
				if (ROOM_MAP[x][y] == ROOM_COUNT) {
					//We need to pick a position here!
					printf("Room @ %i, %i (%i, array says %i)\n", x, y, ROOM_COUNT, ROOM_MAP[x][y]);
				}

				if (ROOM_MAP[x][y] > ROOM_COUNT) {
					DIJKSTRA_MAP[x][y] = -1;
				} else if (ROOM_MAP[x][y] > 0 && ROOM_MAP[x][y] < ROOM_COUNT) {
					DIJKSTRA_MAP[x][y] = 0;
				} else if (!ROOM_MAP[x][y]) {
					DIJKSTRA_MAP[x][y] = 1000;
				}
			}
		}

		mapUpdates = 1;

		while (mapUpdates) {
			mapUpdates = 0;

			for (y = 2; y < WINDOW_HEIGHT - 1; y++) {
				for (x = 2; x < WINDOW_WIDTH - 1; x++) {
					lowestValue = 1001;
					currentValue = DIJKSTRA_MAP[x][y];

					if (currentValue <= 0) {
						continue;
					}

					for (y1 = -1; y1 <= 1; y1++) {
						for (x1 = -1; x1 <= 1; x1++) {
							if ((x1 == 0 && y1 == 0) | (x + x1 <= 2 || x + x1 >= WINDOW_WIDTH - 2 || y + y1 <= 2 || y + y1 >= WINDOW_HEIGHT - 2)) {
								continue;
							}

							neighborValue = DIJKSTRA_MAP[x + x1][y + y1];

							if (neighborValue == -1) {
								continue;
							}

							if (neighborValue >= currentValue) {
								continue;
							} else if (neighborValue - 1 < lowestValue) {
								lowestValue = neighborValue + 1;
							}
						}
					}

					if (lowestValue < currentValue) {
						DIJKSTRA_MAP[x][y] = lowestValue;

						mapUpdates ++;
					}
				}
			}
		}

		printf("FORCING A BREAK!!!!!!!!!!!!!\n");

		for (y = 2; y < WINDOW_HEIGHT - 1; y++) {
			for (x = 2; x < WINDOW_WIDTH - 1; x++) {
				//printf("%-2i", DIJKSTRA_MAP[x][y]);
				printf("%i", ROOM_MAP[x][y]);
			}

			printf("\n");
		}

		break;

		ROOM_COUNT --;
	}
}

void generateLevel() {
	int x, y, i;
	float fogValue, colorMod;
	float p[2];
	TCOD_noise_t fog = getFogNoise();
	
	for (i = 0; i < 6; i++) {
		if (!i) {
			x = WINDOW_WIDTH / 2;
			y = WINDOW_HEIGHT / 2;
		} else {
			x = TCOD_random_get_int(RANDOM, 16, WINDOW_WIDTH - 16);
			y = TCOD_random_get_int(RANDOM, 16, WINDOW_HEIGHT - 16);
		}

		carve(x, y);
	}

	smooth();
	//placeLights();
	findRooms();
	placeTunnels();
	
	drawLights();

	createBonfire((WINDOW_WIDTH/2) - 1, WINDOW_HEIGHT/2);
	
	for (y = 0; y < WINDOW_HEIGHT; y++) {
		for (x = 0; x < WINDOW_WIDTH; x++) {
			p[0] = (float) x / WINDOW_WIDTH;
			p[1] = (float) y / WINDOW_HEIGHT;

			fogValue = TCOD_noise_get_fbm_ex(fog, p, 32.0f, TCOD_NOISE_PERLIN) + .2f;

			if (fogValue < 0) {
				fogValue = 0;
			}

			if (fogValue > .6) {
				fogValue = .6;
			}

			if (!TCOD_map_is_walkable(LEVEL_MAP, x, y)) {
				drawCharBackEx(LEVEL_CONSOLE, x, y, TCOD_color_RGB(44, 8, 8), TCOD_BKGND_SET);
			} else {
				colorMod = (int)(fogValue * 150);
				drawCharBackEx(FOG_CONSOLE, x, y, TCOD_color_RGB(135 - colorMod, 100 - colorMod, 100 - colorMod), TCOD_BKGND_ALPHA(1));
			}
		}
	}
}
