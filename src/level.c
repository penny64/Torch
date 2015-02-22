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
TCOD_map_t TUNNEL_MAP;
TCOD_map_t TUNNEL_WALLS;
TCOD_noise_t FOG_NOISE;
TCOD_random_t RANDOM;
int (*ROOM_MAP)[255];
int (*TUNNEL_ROOM_MAP)[255];
int (*DIJKSTRA_MAP)[255];
int (*CLOSED_MAP)[255];
float (*EFFECTS_MAP)[255];
float EXIT_WAVE_DIST;
int ROOM_COUNT, ROOM_COUNT_MAX;
int EXIT_OPEN = 0;
int EXIT_IN_PROGRESS;
int EXIT_LOCATION[2];
int LEVEL_NUMBER;
int (*openList)[WINDOW_WIDTH * WINDOW_HEIGHT];
int (*START_POSITIONS)[WINDOW_WIDTH * WINDOW_HEIGHT];
room *ROOMS = NULL;


void levelSetup() {
	LEVEL_CONSOLE = TCOD_console_new(WINDOW_WIDTH, WINDOW_HEIGHT);
	SHADOW_CONSOLE = TCOD_console_new(WINDOW_WIDTH, WINDOW_HEIGHT);
	FOG_CONSOLE = TCOD_console_new(WINDOW_WIDTH, WINDOW_HEIGHT);
	SEEN_CONSOLE = TCOD_console_new(WINDOW_WIDTH, WINDOW_HEIGHT);
	LIGHT_CONSOLE = TCOD_console_new(WINDOW_WIDTH, WINDOW_HEIGHT);
	LEVEL_MAP = TCOD_map_new(WINDOW_WIDTH, WINDOW_HEIGHT);
	TUNNEL_MAP = TCOD_map_new(WINDOW_WIDTH, WINDOW_HEIGHT);
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
	TUNNEL_ROOM_MAP = calloc(1, sizeof(double[255][255]));
	EFFECTS_MAP = calloc(1, sizeof(float[255][255]));
	openList = calloc(1, sizeof(double[WINDOW_WIDTH * WINDOW_HEIGHT][WINDOW_WIDTH * WINDOW_HEIGHT]));
	DIJKSTRA_MAP = malloc(sizeof(double[255][255]));
	START_POSITIONS = malloc(sizeof(double[WINDOW_WIDTH * WINDOW_HEIGHT][WINDOW_WIDTH * WINDOW_HEIGHT]));
	LEVEL_NUMBER = 1;
	
	startLights();
}

void levelShutdown() {
	printf("Shutting down level...\n");
	
	deleteAllRooms();
	free(ROOM_MAP);
	free(TUNNEL_ROOM_MAP);
	free(EFFECTS_MAP);
	free(CLOSED_MAP);
	free(START_POSITIONS);
	free(DIJKSTRA_MAP);
	free(openList);
	TCOD_console_delete(LEVEL_CONSOLE);
	TCOD_console_delete(LIGHT_CONSOLE);
	TCOD_console_delete(SHADOW_CONSOLE);
	TCOD_console_delete(FOG_CONSOLE);
	TCOD_console_delete(SEEN_CONSOLE);
	TCOD_map_delete(LEVEL_MAP);
	TCOD_map_delete(TUNNEL_MAP);
	TCOD_map_delete(TUNNEL_WALLS);
	TCOD_noise_delete(FOG_NOISE);
}

room *createRoom(int id, int roomSize, unsigned int flags) {
	room *ptr, *rm = calloc(1, sizeof(room));
	int x, y, i, positionIndex = 0;

	rm->id = id;
	rm->size = roomSize;
	rm->numberOfConnectedRooms = 0;
	rm->flags = flags;
	rm->prev = NULL;
	rm->next = NULL;
	rm->connectedRooms = (int*)malloc(MAX_ROOMS * sizeof(int));

	printf("FLAGS %i %i  size=%i\n", rm->flags, flags, roomSize);

	//TODO: Use memcpy in the future
	rm->positionList = malloc(sizeof *rm->positionList * roomSize);
	if (rm->positionList)
	{
		for (i = 0; i < roomSize; i++)
		{
			rm->positionList[i] = malloc(sizeof(int) * 2);
		}
	}

	for (y = 0; y < WINDOW_HEIGHT; y ++) {
		for (x = 0; x < WINDOW_WIDTH; x ++) {
			if (ROOM_MAP[x][y] == id) {
				rm->positionList[positionIndex][0] = x;
				rm->positionList[positionIndex][1] = y;

				positionIndex ++;
			}
		}
	}
	
	if (ROOMS == NULL) {
		ROOMS = rm;
	} else {
		ptr = ROOMS;
		
		while (ptr->next) {
			ptr = ptr->next;
		}
		
		ptr->next = rm;
		rm->prev = ptr;
	}

	return rm;
}

room *getRoomViaId(int id) {
	room *roomPtr = ROOMS;
			
	while (roomPtr) {
		if (roomPtr->id == id) {
			return roomPtr;
		}
		
		roomPtr = roomPtr->next;
	}
	
	printf("FATAL: Failed to return room of ID %i\n", id);
	
	return NULL;
}

void deleteRoom(room *rm) {
	if (rm == ROOMS) {
		ROOMS = rm->next;
	} else {
		rm->prev->next = rm->next;

		if (rm->next) {
			rm->next->prev = rm->prev;
		}
	}

	free(rm->connectedRooms);
	free(rm);
}

void deleteAllRooms() {
	room *next, *ptr = ROOMS;

	printf("Deleting all rooms...\n");

	while (ptr != NULL) {
		next = ptr->next;

		deleteRoom(ptr);

		ptr = next;
	}
	
	ROOMS = NULL; //Just in case...?
}

void connectRooms(room *srcRoom, room *dstRoom) {
	if (!isRoomConnectedTo(srcRoom, dstRoom)) {
		srcRoom->connectedRooms[srcRoom->numberOfConnectedRooms] = dstRoom->id;
		
		srcRoom->numberOfConnectedRooms ++;
	}
	
	if (!isRoomConnectedTo(dstRoom, srcRoom)) {
		dstRoom->connectedRooms[dstRoom->numberOfConnectedRooms] = srcRoom->id;
		
		dstRoom->numberOfConnectedRooms ++;
	}
}

int isRoomConnectedTo(room *srcRoom, room *dstRoom) {
	int i;

	for (i = 0; i <= srcRoom->numberOfConnectedRooms; i++) {
		if (srcRoom->connectedRooms[i] == dstRoom->id) {
			return 1;
		}
	}

	return 0;
}

int isRoomConnectedToId(room *srcRoom, int dstRoomId) {
	int i;

	for (i = 0; i <= srcRoom->numberOfConnectedRooms; i++) {
		if (srcRoom->connectedRooms[i] == dstRoomId) {
			return 1;
		}
	}

	return 0;
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
					//drawCharBackEx(LEVEL_CONSOLE, x, y, TCOD_color_RGB(155 + RED_SHIFT, 255, 155), TCOD_BKGND_SET);
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
					//drawCharBackEx(LEVEL_CONSOLE, x, y, TCOD_color_RGB(35 + RED_SHIFT, 155, 205), TCOD_BKGND_SET);
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
			TUNNEL_ROOM_MAP[x][y] = 0;
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

			for (i = 0; i < oLen; i++) {
				x = openList[i][0];
				y = openList[i][1];

				ROOM_MAP[x][y] = ROOM_COUNT;
			}

			if (ROOM_COUNT == ROOM_COUNT_MAX) {
				createRoom(ROOM_COUNT, oLen, IS_TREASURE_ROOM);
			} else if (getRandomInt(0, 2)) {
				createRoom(ROOM_COUNT, oLen, IS_TORCH_ROOM);
			} else {
				createRoom(ROOM_COUNT, oLen, 0x00);
			}

			printf("Found new room: %i (%i)\n", ROOM_COUNT, oLen);
		}
	}

	ROOM_COUNT_MAX = ROOM_COUNT;
}

void placeTunnels() {
	int x, y, x1, y1, w_x, w_y, prev_w_x, prev_w_y, tunnelPlaced, mapUpdates, currentValue, neighborValue, lowestValue, index, lowestX, lowestY, invalid, randomRoomSize, dist;
	int neighborCollision, banDoubleTunnels, srcRoomIndex, dstRoomIndex, startCount = 0, runCount = -1;
	int ownsTunnels;//, openRoomList[MAX_ROOMS], closedRoomList[MAX_ROOMS], openListCount, closedListCount, inClosedList, inOpenList, i, ii, id;
	room *tempRoom, *srcRoom = NULL, *dstRoom = NULL, *roomPtr;
	
	ROOM_COUNT = ROOM_COUNT_MAX;
	
	while (1) {
		runCount ++;
		startCount = 0;
		ownsTunnels = 0;
		banDoubleTunnels = getRandomInt(0, 2);
		
		for (y = 2; y < WINDOW_HEIGHT - 1; y++) {
			for (x = 2; x < WINDOW_WIDTH - 1; x++) {
				DIJKSTRA_MAP[x][y] = 0;
			}
		}

		srcRoom = NULL;
		dstRoom = NULL;
		srcRoomIndex = -1;
		dstRoomIndex = -1;
		
		while (1) {
			//while (srcRoomIndex == dstRoomIndex) {
			srcRoomIndex = getRandomInt(1, ROOM_COUNT_MAX);
			srcRoom = getRoomViaId(srcRoomIndex);
			//}
			
			//while (dstRoomIndex == -1 )
			while (dstRoomIndex == -1 || isRoomConnectedToId(srcRoom, dstRoomIndex) || (dstRoom->flags & IS_TREASURE_ROOM && dstRoom->numberOfConnectedRooms)) {
				dstRoomIndex = getRandomInt(1, ROOM_COUNT_MAX);
				dstRoom = getRoomViaId(dstRoomIndex);
			}
			//dstRoomIndex = getRandomInt(1, ROOM_COUNT_MAX);
			
			
			
			
			printf("SHIT\n");
			
			//while (isRoomConnectedToId(dstRoom, srcRoomIndex)) {
			//	srcRoomIndex = getRandomInt(1, ROOM_COUNT_MAX);
				
			//	continue;
			//}
			
			break;
		}

		if (srcRoom == NULL) {
			printf("CRASH: No src room\n");
		}
		
		if (dstRoom == NULL) {
			printf("CRASH: No dst room\n");
		}
		
		if (srcRoom->flags & IS_TREASURE_ROOM || dstRoom->flags & IS_TREASURE_ROOM) {
			banDoubleTunnels = 1;
		}
		
		for (y = 2; y < WINDOW_HEIGHT - 1; y++) {
			for (x = 2; x < WINDOW_WIDTH - 1; x++) {
				if (TUNNEL_ROOM_MAP[x][y] == srcRoom->id) {
					ownsTunnels = 1;
					
					break;
				}
			}
			
			if (ownsTunnels) {
				break;
			}
		}

		//Find our first room
		for (y = 2; y < WINDOW_HEIGHT - 1; y++) {
			for (x = 2; x < WINDOW_WIDTH - 1; x++) {
				invalid = 0;
				neighborCollision = 0;
				
				for (y1 = -1; y1 <= 1; y1++) {
					for (x1 = -1; x1 <= 1; x1++) {
						if ((y1 == -1 && x1 == 1) || (y1 == -1 && x1 == -1) || (y1 == 1 && x1 == 1) || (y1 == 1 && x1 == -1)) {
							continue;
						}
						
						if (!ROOM_MAP[x + x1][y + y1] && (TUNNEL_ROOM_MAP[x + x1][y + y1] != srcRoom->id || TUNNEL_ROOM_MAP[x + x1][y + y1] != dstRoom->id)) {
							invalid = 1;
							
							break;
						}
					}
					
					if (invalid) {
						break;
					}
				}
				
				
				//TODO: Find center?
				if (!invalid) {
					if (ROOM_MAP[x][y] == srcRoom->id || TUNNEL_ROOM_MAP[x][y] == srcRoom->id) {
						START_POSITIONS[startCount][0] = x;
						START_POSITIONS[startCount][1] = y;

						startCount ++;
					}
				}
				
				for (y1 = -1; y1 <= 1; y1++) {
					for (x1 = -1; x1 <= 1; x1++) {
						if (x1 == 0 && y1 == 0) {
							continue;
						}
						
						if ((banDoubleTunnels && TCOD_map_is_walkable(TUNNEL_MAP, x + x1, y + y1)) || (ROOM_MAP[x + x1][y + y1] && (ROOM_MAP[x + x1][y + y1] != srcRoom->id && ROOM_MAP[x + x1][y + y1] != dstRoom->id))) {
							neighborCollision = 1;
						}
					}
				}
				
				if (neighborCollision) {
					DIJKSTRA_MAP[x][y] = -1;
				} else {
					if (ROOM_MAP[x][y] == dstRoom->id || TUNNEL_ROOM_MAP[x][y] == dstRoom->id) {
						DIJKSTRA_MAP[x][y] = 0;
					} else if ((ROOM_MAP[x][y] && ROOM_MAP[x][y] != dstRoom->id && ROOM_MAP[x][y] != srcRoom->id) || TCOD_map_is_walkable(TUNNEL_MAP, x, y)) {
						DIJKSTRA_MAP[x][y] = -1;
					} else {
						DIJKSTRA_MAP[x][y] = 99;
					}
				}
			}
		}
		
		index = TCOD_random_get_int(RANDOM, 0, startCount - 1);
		w_x = START_POSITIONS[index][0];
		w_y = START_POSITIONS[index][1];
		prev_w_x = w_x;
		prev_w_y = w_y;
		
		//printf("Starting at %i, %i VAL=%i rm=%i, sr=%i, dr=%i\n", w_x, w_y, DIJKSTRA_MAP[w_x][w_y], ROOM_MAP[w_x][w_y], srcRoom->id, dstRoom->id);

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
					
					if (DIJKSTRA_MAP[w_x + x1][w_y + y1] == -1) {
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

			//printf("Walking to %i, %i (lowest=%i) (prev=%i, %i)\n", w_x, w_y, lowestValue, prev_w_x, prev_w_y);
			
			randomRoomSize = 1;//clip(TCOD_random_get_int(RANDOM, minRoomSize, maxRoomSize), 1, 255);
			
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
					
					//if (dist <= randomRoomSize && !TCOD_map_is_walkable(LEVEL_MAP, w_x + x1, w_y + y1)) {
					//	drawCharBackEx(LEVEL_CONSOLE, w_x + x1, w_y + y1, TCOD_color_RGB(15 + RED_SHIFT, 105, 155), TCOD_BKGND_SET);
					//}
					
					if (!TCOD_map_is_walkable(LEVEL_MAP, w_x + x1, w_y + y1)) {
						TCOD_map_set_properties(TUNNEL_MAP, w_x + x1, w_y + y1, 1, 1);
						TUNNEL_ROOM_MAP[w_x + x1][w_y + y1] = srcRoom->id;
					}
					
					TCOD_map_set_properties(LEVEL_MAP, w_x + x1, w_y + y1, 1, 1);

					tunnelPlaced = 1;
				}
			}
		}

		if (tunnelPlaced && randomRoomSize == 1) {
			createDoor(prev_w_x, prev_w_y);
		}

		printf("Connecting rooms: %i, %i\n", srcRoom->id, dstRoom->id);

		connectRooms(srcRoom, dstRoom);
		
		printf("%i, %i id=%i\n", srcRoom->numberOfConnectedRooms, ROOM_COUNT_MAX, srcRoom->id);

		if (srcRoom->numberOfConnectedRooms == ROOM_COUNT_MAX - 1) {
			break;
		}
	}
}

void placeItemChest() {

}

void generatePuzzles() {
	int x, y, spawnIndex;
	room *roomPtr = ROOMS;

	while (roomPtr) {
		if (roomPtr->flags & IS_TORCH_ROOM) {
			spawnIndex = getRandomInt(0, roomPtr->size - 1);
			x = roomPtr->positionList[spawnIndex][0];
			y = roomPtr->positionList[spawnIndex][1];

			createBonfireKeystone(x, y);
		}

		roomPtr = roomPtr->next;
	}

	placeItemChest();
}

void cleanUpDoors() {
	int x1, y1, closedCount, openCount;
	item *next, *itm = getItems();

	while (itm) {
		next = itm->next;
		openCount = 0;
		closedCount = 0;

		if (!(itm->itemFlags & IS_DOOR)) {
			itm = next;

			continue;
		}

		for (x1 = -1; x1 <= 1; x1 ++) {
			for (y1 = -1; y1 <= 1; y1 ++) {
				if (!x1 && !y1) {
					continue;
				}
				
				if (!(y1 == -1 && x1 == 1) || (y1 == -1 && x1 == -1) || (y1 == 1 && x1 == 1) || (y1 == 1 && x1 == -1)) {
					if (TCOD_map_is_walkable(LEVEL_MAP, itm->x + x1, itm->y + y1)) {
						openCount ++;
					}
				} else {
					if (TCOD_map_is_walkable(LEVEL_MAP, itm->x + x1, itm->y + y1)) {
						closedCount ++;
					}
				}
			}
		}

		if (closedCount == 2 || openCount > 3) {
			deleteItem(itm);
		} else {
			printf("%i %i @ %i %i\n", openCount, closedCount, itm->x, itm->y);
		}

		itm = next;
	}
}

void activateDoors() {
	item *itm = getItems();

	while (itm) {
		if (itm->itemFlags & IS_DOOR) {
			enableDoor(itm);
		}
		
		itm = itm->next;
	}
}

void blockPosition(int x, int y) {
	TCOD_map_set_properties(LEVEL_MAP, x, y, 0, 0);
	
	resetAllActorsForNewLevel();
}

void unblockPosition(int x, int y) {
	TCOD_map_set_properties(LEVEL_MAP, x, y, 1, 1);
	
	resetAllActorsForNewLevel();
}

void colorRooms() {
	int i, x, y, r, g, b, rMod, gMod, bMod;
	room *roomPtr = ROOMS;

	while (roomPtr) {
		//printf("FLAGS: %i, %i\n", roomPtr->flags, 0x01 << 1);

		rMod = 0;
		gMod = 0;
		bMod = 0;

		if (roomPtr->flags & IS_TREASURE_ROOM) {
			r = 235;
			g = 105;
			b = 255;

			gMod = 70;
			printf("1\n");
		} else if (roomPtr->flags & IS_TORCH_ROOM) {
			r = 140 - RED_SHIFT;
			g = 140;
			b = 140;

			rMod = 70;
			gMod = 70;
			bMod = 70;
			printf("2\n");
		}
		else if (roomPtr->numberOfConnectedRooms > 3) {
			r = 15;
			g = 205;
			b = 255;
			rMod = 120;
			gMod = 120;
			printf("3\n");
		} else {
			r = 205;
			g = 25;
			b = 25;
			printf("4\n");
		}

		for (i = 0; i < roomPtr->size; i ++) {
			x = roomPtr->positionList[i][0];
			y = roomPtr->positionList[i][1];

			/*if (roomPtr->numberOfConnectedRooms > 2) {

				printf("%i, %i\n", x, y);

				if (!y % 2) {
					printf("FUCKKKKKKK!, %i\n", y % 2);
					rMod = 0;
					bMod = 250;
					gMod = 255;
				} else {
					rMod = 0;
					bMod = 0;
					gMod = 0;
				}
			}*/

			drawCharBackEx(LEVEL_CONSOLE, x, y, TCOD_color_RGB(clip(r + RED_SHIFT + getRandomInt(0, rMod), 0, 255), clip(g + getRandomInt(0, gMod), 0, 255), clip(b + getRandomInt(0, bMod), 0, 255)), TCOD_BKGND_SET);
		}

		for (y = 2; y < WINDOW_HEIGHT - 2; y ++) {
			for (x = 2; x < WINDOW_WIDTH - 2; x ++) {
				if (TUNNEL_ROOM_MAP[x][y] == roomPtr->id) {
					drawCharBackEx(LEVEL_CONSOLE, x, y, TCOD_color_RGB(200 + RED_SHIFT, 82, 45), TCOD_BKGND_SET);
				}
			}
		}

		roomPtr = roomPtr->next;
	}
}

void generateLevel() {
	int x, y, i, ii, spawnIndex, foundPlot, plotDist, plotPoints[MAX_ROOMS][2];
	float fogValue, colorMod;
	float p[2];
	room *roomPtr, *startingRoom = NULL;
	TCOD_noise_t fog = getFogNoise();
	TCOD_console_t dynamicLightConsole = getDynamicLightConsole();
	character *player = getPlayer();

	EXIT_OPEN = 0;
	EXIT_WAVE_DIST = 0;

	TCOD_map_clear(LEVEL_MAP, 0, 0);
	TCOD_map_clear(TUNNEL_MAP, 0, 0);
	TCOD_map_clear(TUNNEL_WALLS, 0, 0);
	TCOD_console_clear(LEVEL_CONSOLE);
	TCOD_console_clear(LIGHT_CONSOLE);
	TCOD_console_clear(SHADOW_CONSOLE);
	TCOD_console_clear(SEEN_CONSOLE);
	//TCOD_console_clear(FOG_CONSOLE);

	if (LEVEL_NUMBER > 1) {
		TCOD_console_clear(dynamicLightConsole);
	}

	deleteAllRooms();
	deleteEnemies();
	deleteAllOwnerlessItems();
	
	for (i = 0; i <= MAX_ROOMS; i++) {
		foundPlot = 0;

		while (!foundPlot) {
			foundPlot = 1;

			x = TCOD_random_get_int(RANDOM, 8, WINDOW_WIDTH - 8);
			y = TCOD_random_get_int(RANDOM, 8, WINDOW_HEIGHT - 8);

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
	activateDoors();
	generatePuzzles();

	drawLights();
	drawDynamicLights();
	colorRooms();
	
	if (player) {
		roomPtr = ROOMS;

		while (roomPtr) {
			if (!startingRoom || roomPtr->numberOfConnectedRooms > startingRoom->numberOfConnectedRooms) {
				startingRoom = roomPtr;
			}

			roomPtr = roomPtr->next;
		}
		spawnIndex = getRandomInt(0, startingRoom->size - 1);
		player->x = startingRoom->positionList[spawnIndex][0];
		player->y = startingRoom->positionList[spawnIndex][1];
		player->vx = 1;
		
		printf("Spawning at %i, %i\n", player->x, player->y);
	}

	if (player) {
		if (LEVEL_NUMBER == 1) {
			plantTorch(player);
			//createVoidWorm(player->x + 1, player->y + 1);
			//showMessage("%cSomething watches from the shadows...%c", 10);
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
