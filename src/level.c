#include <stdlib.h>
#include <stdio.h>

#include "framework/display.h"
#include "framework/numbers.h"
#include "framework/draw.h"
#include "graphics.h"
#include "level.h"
#include "lights.h"
#include "libtcod.h"
#include "items.h"
#include "player.h"
#include "enemies.h"
#include "ui.h"


TCOD_console_t LEVEL_CONSOLE;
TCOD_console_t LIGHT_CONSOLE;
TCOD_console_t SHADOW_CONSOLE;
TCOD_console_t FOG_CONSOLE;
TCOD_console_t SEEN_CONSOLE;
TCOD_map_t LEVEL_MAP;
TCOD_map_t TUNNEL_WALLS;
TCOD_noise_t FOG_NOISE;
TCOD_random_t RANDOM;
int (*ROOM_MAP)[255];
int (*DIJKSTRA_MAP)[255];
int (*CLOSED_MAP)[255];
float (*EFFECTS_MAP)[255];
float EXIT_WAVE_DIST;
int ROOM_COUNT, ROOM_COUNT_MAX;
int EXIT_OPEN = 0;
int EXIT_IN_PROGRESS;
int EXIT_LOCATION[2];
int LEVEL_NUMBER;
int KEY_TORCH_COUNT, (*KEY_TORCH_POSITIONS)[255];
int (*openList)[WINDOW_WIDTH * WINDOW_HEIGHT];
int (*START_POSITIONS)[WINDOW_WIDTH * WINDOW_HEIGHT];


void levelSetup() {
	LEVEL_CONSOLE = TCOD_console_new(WINDOW_WIDTH, WINDOW_HEIGHT);
	SHADOW_CONSOLE = TCOD_console_new(WINDOW_WIDTH, WINDOW_HEIGHT);
	FOG_CONSOLE = TCOD_console_new(WINDOW_WIDTH, WINDOW_HEIGHT);
	SEEN_CONSOLE = TCOD_console_new(WINDOW_WIDTH, WINDOW_HEIGHT);
	LIGHT_CONSOLE = TCOD_console_new(WINDOW_WIDTH, WINDOW_HEIGHT);
	LEVEL_MAP = TCOD_map_new(WINDOW_WIDTH, WINDOW_HEIGHT);
	TUNNEL_WALLS = TCOD_map_new(WINDOW_WIDTH, WINDOW_HEIGHT);
	FOG_NOISE = TCOD_noise_new(2, TCOD_NOISE_DEFAULT_HURST, TCOD_NOISE_DEFAULT_LACUNARITY, NULL);
	RANDOM = TCOD_random_get_instance();

	TCOD_console_set_default_background(LEVEL_CONSOLE, TCOD_color_RGB(40, 30, 30));
	TCOD_console_set_default_background(LIGHT_CONSOLE, TCOD_color_RGB(0, 0, 0));
	TCOD_console_set_default_background(SHADOW_CONSOLE, TCOD_color_RGB(0, 0, 0));
	TCOD_console_set_default_background(SEEN_CONSOLE, TCOD_color_RGB(15, 15, 15));
	TCOD_console_set_key_color(LIGHT_CONSOLE, TCOD_color_RGB(255, 0, 255));
	TCOD_console_set_key_color(SHADOW_CONSOLE, TCOD_color_RGB(0, 0, 0));
	TCOD_console_set_key_color(SEEN_CONSOLE, TCOD_color_RGB(255, 0, 255));

	TCOD_console_clear(LEVEL_CONSOLE);
	TCOD_console_clear(LIGHT_CONSOLE);
	TCOD_console_clear(SHADOW_CONSOLE);
	TCOD_console_clear(SEEN_CONSOLE);
	
	TCOD_noise_set_type(FOG_NOISE, TCOD_NOISE_PERLIN);

	CLOSED_MAP = calloc(1, sizeof(double[255][255]));
	ROOM_MAP = calloc(1, sizeof(double[255][255]));
	EFFECTS_MAP = calloc(1, sizeof(float[255][255]));
	openList = calloc(1, sizeof(double[WINDOW_WIDTH * WINDOW_HEIGHT][WINDOW_WIDTH * WINDOW_HEIGHT]));
	DIJKSTRA_MAP = malloc(sizeof(double[255][255]));
	START_POSITIONS = malloc(sizeof(double[WINDOW_WIDTH * WINDOW_HEIGHT][WINDOW_WIDTH * WINDOW_HEIGHT]));
	KEY_TORCH_POSITIONS = calloc(1, sizeof(double[12][12]));
	LEVEL_NUMBER = 1;
	
	startLights();
}

void levelShutdown() {
	printf("Shutting down level...\n");
	
	free(ROOM_MAP);
	free(EFFECTS_MAP);
	free(CLOSED_MAP);
	free(START_POSITIONS);
	free(DIJKSTRA_MAP);
	free(openList);
	free(KEY_TORCH_POSITIONS);
	TCOD_console_delete(LEVEL_CONSOLE);
	TCOD_console_delete(LIGHT_CONSOLE);
	TCOD_console_delete(SHADOW_CONSOLE);
	TCOD_console_delete(FOG_CONSOLE);
	TCOD_console_delete(SEEN_CONSOLE);
	TCOD_map_delete(LEVEL_MAP);
	TCOD_map_delete(TUNNEL_WALLS);
	TCOD_noise_delete(FOG_NOISE);
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

float *getEffectsMap() {
	return *EFFECTS_MAP;
}

float getExitWaveDistance() {
	return EXIT_WAVE_DIST;
}

int *getExitLocation() {
	return EXIT_LOCATION;
}

int isPositionWalkable(int x, int y) {
	return TCOD_map_is_walkable(LEVEL_MAP, x, y);
}

void completeLevel() {
	EXIT_OPEN = 1;

	createExit(EXIT_LOCATION[0], EXIT_LOCATION[1]);

	showMessage("%cYou hear a low rumble.%c", 20);
}

void exitLevel() {
	EXIT_IN_PROGRESS = 1;
	LEVEL_NUMBER ++;

	showMessage("%cYou step down...%c", 20);
}

void setLevel(int levelNumber) {
	LEVEL_NUMBER = levelNumber;
}

int getLevel() {
	return LEVEL_NUMBER;
}

int isLevelComplete() {
	return EXIT_OPEN;
}

void transitionIsComplete() {
	EXIT_IN_PROGRESS = 0;
}

int isTransitionInProgress() {
	return EXIT_IN_PROGRESS;
}

int levelLogic() {
	character *player = getPlayer();

	if (isLevelComplete()) {
		EXIT_WAVE_DIST = clipFloat(EXIT_WAVE_DIST + .5f, 0, 255);

		if (!player->itemLight->sizeMod && isScreenFadedOut()) {
			generateLevel();

			return 1;
		}
	}

	return 0;
}

void carve(int x, int y) {
	int x1, y1, x2, y2, nx, ny, xMod, yMod, invalidPos, lastXMod = -3, lastYMod = -3;
	int i, ii;

	TCOD_map_t existingLevel = copyLevelMap();
	
	for (i = 0; i < TCOD_random_get_int(RANDOM, 9, 12); i++) {
		for (ii = 0; ii <= 0; ii++) { //Useless
			for (y1 = -1 - ii; y1 <= 1 + ii; y1++) {
				for (x1 = -1 - ii; x1 <= 1 + ii; x1++) {
					if (x + x1 <= 1 || x + x1 >= WINDOW_WIDTH - 2 || y + y1 <= 1 || y + y1 >= WINDOW_HEIGHT - 2) {
						continue;
					}

					invalidPos = 0;

					for (y2 = -1; y2 <= 1; y2++) {
						for (x2 = -1; x2 <= 1; x2++) {
							nx = x + x1 + x2;
							ny = y + y1 + y2;

							if (TCOD_map_is_walkable(existingLevel, nx, ny)) {
								invalidPos = 1;

								break;
							}
						}

						if (invalidPos) {
							break;
						}
					}

					if (invalidPos) {
						continue;
					}
					
					TCOD_map_set_properties(LEVEL_MAP, x + x1, y + y1, 1, 1);
					drawCharBackEx(LEVEL_CONSOLE, x, y, TCOD_color_RGB(155 + RED_SHIFT, 255, 155), TCOD_BKGND_SET);
				}
			}
		}
		
		//if (!i && !TCOD_random_get_int(RANDOM, 0, 3)) {
		//	createTreasure(x, y);
		//}
		
		while (TCOD_map_is_walkable(LEVEL_MAP, x, y)) {
			xMod = TCOD_random_get_int(RANDOM, -1, 1);

			while (xMod == lastXMod) {
				xMod = TCOD_random_get_int(RANDOM, -1, 1);
			}

			x += xMod;
			lastXMod = xMod;

			yMod = TCOD_random_get_int(RANDOM, -1, 1);

			while (yMod == lastYMod) {
				yMod = TCOD_random_get_int(RANDOM, -1, 1);
			}

			y += yMod;
			lastYMod = yMod;
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
					drawCharBackEx(LEVEL_CONSOLE, x, y, TCOD_color_RGB(35 + RED_SHIFT, 155, 205), TCOD_BKGND_SET);
				}
			}
		}
		
		TCOD_map_delete(mapCopy);
	}
}

void findRooms() {
	int i, x, y, x1, y1, w_x, w_y, oLen, cLen, added = 1;
	ROOM_COUNT = 0;

	for (y = 0; y <= WINDOW_HEIGHT; y++) {
		for (x = 0; x <= WINDOW_WIDTH; x++) {
			CLOSED_MAP[x][y] = 0;
			ROOM_MAP[x][y] = 0;
			EFFECTS_MAP[x][y] = 0.1f;
			openList[x][y] = 0;
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
					
					if ((y1 == -1 && x1 == 1) || (y1 == -1 && x1 == -1) || (y1 == 1 && x1 == 1) || (y1 == 1 && x1 == -1)) {
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

			printf("Found new room: %i (%i)\n", ROOM_COUNT, oLen);
		}

		for (i = 0; i < oLen; i++) {
			x = openList[i][0];
			y = openList[i][1];
			
			ROOM_MAP[x][y] = ROOM_COUNT;
		}
	}
	
	ROOM_COUNT_MAX = ROOM_COUNT;
}

void placeTunnels() {
	int i, x, y, x1, y1, w_x, w_y, prev_w_x, prev_w_y, tunnelPlaced, mapUpdates, currentValue, neighborValue, lowestValue, index, lowestX, lowestY, invalid, minRoomSize, maxRoomSize, randomRoomSize, dist;
	int startCount = 0;

	KEY_TORCH_COUNT = 0;
	
	for (y = 2; y < WINDOW_HEIGHT - 1; y++) {
		for (x = 2; x < WINDOW_WIDTH - 1; x++) {
			DIJKSTRA_MAP[x][y] = 0;
		}
	}

	minRoomSize = 0;
	maxRoomSize = TCOD_random_get_int(RANDOM, 0, 1);
	
	//TODO: Adjust max for more "connected" levels
	for (i = 0; i <= 1; i++) {
		ROOM_COUNT = ROOM_COUNT_MAX;
		
		while (ROOM_COUNT >= 1) {
			if (i) {
				printf("Generating random tunnels for room %i\n", ROOM_COUNT);
			} else {
				printf("Generating tunnels for room %i\n", ROOM_COUNT);
			}
			
			startCount = 0;

			//Find our first room
			for (y = 2; y < WINDOW_HEIGHT - 1; y++) {
				for (x = 2; x < WINDOW_WIDTH - 1; x++) {
					invalid = 0;
					
					for (y1 = -1; y1 <= 1; y1++) {
						for (x1 = -1; x1 <= 1; x1++) {
							if ((y1 == -1 && x1 == 1) || (y1 == -1 && x1 == -1) || (y1 == 1 && x1 == 1) || (y1 == 1 && x1 == -1)) {
								continue;
							}
							
							if (!ROOM_MAP[x + x1][y + y1]) {
								invalid = 1;
								
								break;
							}
						}
						
						if (invalid) {
							break;
						}
					}
					
					if (!invalid) {
						if (ROOM_MAP[x][y] == ROOM_COUNT) {
							START_POSITIONS[startCount][0] = x;
							START_POSITIONS[startCount][1] = y;
							
							startCount ++;
						}
					}

					if (!i) {
						if (ROOM_MAP[x][y] > ROOM_COUNT) {
							DIJKSTRA_MAP[x][y] = -1;
						} else if (ROOM_MAP[x][y] > 0 && ROOM_MAP[x][y] < ROOM_COUNT) {
							DIJKSTRA_MAP[x][y] = 0;
						//} else if (i && !TCOD_random_get_int(RANDOM, 0, 3)) {
						//	DIJKSTRA_MAP[x][y] = 0;
						} else if (!ROOM_MAP[x][y] || ROOM_MAP[x][y] == ROOM_COUNT) {
							DIJKSTRA_MAP[x][y] = 99;
						}
					} else {
						if (ROOM_MAP[x][y] > 0 && ROOM_MAP[x][y] != ROOM_COUNT && !TCOD_random_get_int(RANDOM, 0, 3)) {
							DIJKSTRA_MAP[x][y] = 0;
						} else if (!ROOM_MAP[x][y] || ROOM_MAP[x][y] == ROOM_COUNT) {
							DIJKSTRA_MAP[x][y] = 99;
						} else {
							DIJKSTRA_MAP[x][y] = -1;
						}
					}
				}
			}
			
			index = TCOD_random_get_int(RANDOM, 0, startCount - 1);
			w_x = START_POSITIONS[index][0];
			w_y = START_POSITIONS[index][1];
			
			if (!i) {
				createBat(w_x, w_y);

				if (ROOM_COUNT == 2) {
					EXIT_LOCATION[0] = w_x;
					EXIT_LOCATION[1] = w_y;
				} else {
					KEY_TORCH_POSITIONS[KEY_TORCH_COUNT][0] = w_x;
					KEY_TORCH_POSITIONS[KEY_TORCH_COUNT][1] = w_y;
					KEY_TORCH_COUNT ++;
				}
			}

			mapUpdates = 1;

			while (mapUpdates) {
				mapUpdates = 0;

				for (y = 2; y < WINDOW_HEIGHT - 1; y++) {
					for (x = 2; x < WINDOW_WIDTH - 1; x++) {
						lowestValue = 99;
						currentValue = DIJKSTRA_MAP[x][y];

						if (currentValue <= 0) {
							continue;
						}

						for (y1 = -1; y1 <= 1; y1++) {
							for (x1 = -1; x1 <= 1; x1++) {
								if ((x1 == 0 && y1 == 0) | (x + x1 <= 2 || x + x1 >= WINDOW_WIDTH - 2 || y + y1 <= 2 || y + y1 >= WINDOW_HEIGHT - 2)) {
									continue;
								}
								
								//if (TCOD_random_get_int(RANDOM, 0, 1)) {
									if ((y1 == -1 && x1 == 1) || (y1 == -1 && x1 == -1) || (y1 == 1 && x1 == 1) || (y1 == 1 && x1 == -1)) {
										continue;
									}
								//}

								neighborValue = DIJKSTRA_MAP[x + x1][y + y1];

								if (neighborValue == -1) {
									continue;
								}

								if (neighborValue >= currentValue) {
									continue;
								} else if (neighborValue - 1 <= lowestValue) {
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

			tunnelPlaced = 0;
			
			while (DIJKSTRA_MAP[w_x][w_y]) {
				lowestValue = DIJKSTRA_MAP[w_x][w_y];
				lowestX = 0;
				lowestY = 0;
				
				for (y1 = -1; y1 <= 1; y1++) {
					for (x1 = -1; x1 <= 1; x1++) {
						if ((x1 == 0 && y1 == 0) | (w_x + x1 <= 2 || w_x + x1 >= WINDOW_WIDTH - 2 || w_y + y1 <= 2 || w_y + y1 >= WINDOW_HEIGHT - 2)) {
							continue;
						}
						
						if ((y1 == -1 && x1 == 1) || (y1 == -1 && x1 == -1) || (y1 == 1 && x1 == 1) || (y1 == 1 && x1 == -1)) {
							continue;
						}
						
						if (DIJKSTRA_MAP[w_x + x1][w_y + y1] < lowestValue) {
							lowestValue = DIJKSTRA_MAP[w_x + x1][w_y + y1];
							lowestX = w_x + x1;
							lowestY = w_y + y1;
						}
					}
				}

				prev_w_x = w_x;
				prev_w_y = w_y;
				w_x = lowestX;
				w_y = lowestY;

				//printf("Walking to %i, %i\n", w_x, w_y);
				
				randomRoomSize = clip(TCOD_random_get_int(RANDOM, minRoomSize, maxRoomSize), 1, 255);
				
				for (y1 = -16; y1 <= 16; y1++) {
					for (x1 = -16; x1 <= 16; x1++) {
						if ((w_x + x1 <= 2 || w_x + x1 >= WINDOW_WIDTH - 2 || w_y + y1 <= 2 || w_y + y1 >= WINDOW_HEIGHT - 2)) {
							continue;
						}

						dist = distance(w_x, w_y, w_x + x1, w_y + y1);
						
						if (dist >= randomRoomSize) {
							if (dist - randomRoomSize <= 4) {
								TCOD_map_set_properties(TUNNEL_WALLS, w_x + x1, w_y + y1, 1, 1);
							}
							
							continue;
						}
						
						if (dist <= randomRoomSize && !TCOD_map_is_walkable(LEVEL_MAP, w_x + x1, w_y + y1)) {
							drawCharBackEx(LEVEL_CONSOLE, w_x + x1, w_y + y1, TCOD_color_RGB(15 + RED_SHIFT, 105, 155), TCOD_BKGND_SET);
						}
						
						TCOD_map_set_properties(LEVEL_MAP, w_x + x1, w_y + y1, 1, 1);

						tunnelPlaced = 1;
					}
				}
			}

			if (tunnelPlaced && randomRoomSize == 1) {
				createDoor(prev_w_x, prev_w_y);
			}

			/*for (y = 2; y < WINDOW_HEIGHT - 1; y++) {
				for (x = 2; x < WINDOW_WIDTH - 1; x++) {
					if (x == START_POSITIONS[index][0] && y == START_POSITIONS[index][1]) {
						printf("X ");
					} else {
						printf("%-2i", DIJKSTRA_MAP[x][y]);
					}
				}

				printf("\n");
			}*/

			ROOM_COUNT --;
		}
	}
}

void placeKeyTorches() {
	int i;

	for (i = 0; i < KEY_TORCH_COUNT; i++) {
		createBonfireKeystone(KEY_TORCH_POSITIONS[i][0], KEY_TORCH_POSITIONS[i][1]);
	}
}

void placeItemChest() {

}

void generatePuzzles() {
	if (ROOM_COUNT_MAX > 4) {
		placeKeyTorches();

		printf("Level type: Key Torches\n");
	} else {

	}

	placeItemChest();
}

void cleanUpDoors() {
	int x1, y1, wallCount;
	item *next, *itm = getItems();

	while (itm) {
		next = itm->next;
		wallCount = 0;

		if (!itm->itemFlags & IS_DOOR) {
			itm = next;

			continue;
		}

		for (x1 = -1; x1 <= 1; x1 ++) {
			for (y1 = -1; y1 <= 1; y1 ++) {
				if ((y1 == -1 && x1 == 1) || (y1 == -1 && x1 == -1) || (y1 == 1 && x1 == 1) || (y1 == 1 && x1 == -1)) {
					continue;
				}

				if (!TCOD_map_is_walkable(LEVEL_MAP, itm->x + x1, itm->y + y1)) {
					wallCount ++;
				}
			}
		}

		if (wallCount < 2) {
			deleteItem(itm);
		}

		itm = next;
	}
}

void generateLevel() {
	int x, y, i, ii, foundPlot, plotDist, plotPoints[MAX_ROOMS][2];
	float fogValue, colorMod;
	float p[2];
	TCOD_noise_t fog = getFogNoise();
	TCOD_console_t dynamicLightConsole = getDynamicLightConsole();
	character *player = getPlayer();

	EXIT_OPEN = 0;
	EXIT_WAVE_DIST = 0;

	TCOD_map_clear(LEVEL_MAP, 0, 0);
	TCOD_map_clear(TUNNEL_WALLS, 0, 0);
	TCOD_console_clear(LEVEL_CONSOLE);
	TCOD_console_clear(LIGHT_CONSOLE);
	TCOD_console_clear(SHADOW_CONSOLE);
	TCOD_console_clear(SEEN_CONSOLE);
	//TCOD_console_clear(FOG_CONSOLE);

	if (LEVEL_NUMBER > 1) {
		TCOD_console_clear(dynamicLightConsole);
	}

	deleteEnemies();
	deleteAllOwnerlessItems();
	
	for (i = 0; i <= MAX_ROOMS; i++) {
		foundPlot = 0;

		while (!foundPlot) {
			foundPlot = 1;

			x = TCOD_random_get_int(RANDOM, 4, WINDOW_WIDTH - 4);
			y = TCOD_random_get_int(RANDOM, 4, WINDOW_HEIGHT - 4);

			for (ii = 0; ii < i; ii++) {
				plotDist = distance(x, y, plotPoints[ii - 1][0], plotPoints[ii - 1][1]);

				if (plotDist <= 14 || plotDist >= 50) {
					foundPlot = 0;
					
					break;
				}
			}
		}

		plotPoints[i - 1][0] = x;
		plotPoints[i - 1][1] = y;

		carve(x, y);
	}

	findRooms();
	placeTunnels();
	smooth();
	cleanUpDoors();
	generatePuzzles();
	
	drawLights();
	drawDynamicLights();
	
	if (player) {
		player->x = plotPoints[0][0];
		player->y = plotPoints[0][1];
		player->vx = 1;
		
		printf("Spawning at %i, %i\n", player->x, player->y);
	}

	if (player) {
		if (LEVEL_NUMBER == 1) {
			plantTorch(player);
			createVoidWorm(player->x + 1, player->y + 1);
			showMessage("%cSomething watches from the shadows...%c", 10);
		} else {
			createBonfire(player->x, player->y);
		}
	}

	resetAllActorsForNewLevel();
	refreshAllLights();
	fadeBackIn();
	
	for (y = 0; y < WINDOW_HEIGHT; y++) {
		for (x = 0; x < WINDOW_WIDTH; x++) {
			p[0] = (float) x / WINDOW_WIDTH;
			p[1] = (float) y / WINDOW_HEIGHT;

			fogValue = TCOD_noise_get_fbm_ex(fog, p, 32.0f, TCOD_NOISE_PERLIN) + .2f;

			if (fogValue < .3) {
				fogValue = .3;
			}

			if (fogValue > .6) {
				fogValue = .6;
			}

			EFFECTS_MAP[x][y] = TCOD_random_get_float(RANDOM, .65, .8);

			if (!TCOD_map_is_walkable(LEVEL_MAP, x, y)) {
				if (TCOD_map_is_walkable(TUNNEL_WALLS, x, y)) {
					drawCharBackEx(LEVEL_CONSOLE, x, y, TCOD_color_RGB(125, 16, 16), TCOD_BKGND_SET);
				} else {
					drawCharBackEx(LEVEL_CONSOLE, x, y, TCOD_color_RGB(95, 8, 8), TCOD_BKGND_SET);
				}
			} else {
				colorMod = (int)(fogValue * 120);
				
				if (!TCOD_random_get_int(RANDOM, 0, 4)) {
					setCharEx(LEVEL_CONSOLE, x, y, ',' + TCOD_random_get_int(RANDOM, 0, 4), TCOD_color_RGB(155 - colorMod, 290 - colorMod, 190 - colorMod));
				}
				
				//drawCharBackEx(FOG_CONSOLE, x, y, TCOD_color_RGB(135 - colorMod, 120 - colorMod, 120 - colorMod), TCOD_BKGND_ALPHA(1));
			}
		}
	}
}
