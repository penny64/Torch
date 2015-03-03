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
	int i, x, y, addPosition, positionIndex = 0, xAvg = 0, yAvg = 0, xAvgLen = 0, yAvgLen = 0;

	rm->id = id;
	rm->size = roomSize;
	rm->numberOfConnectedRooms = 0;
	rm->numberOfDoorPositions = 0;
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
	
	rm->doorPositions = malloc(sizeof *rm->doorPositions * roomSize);
	if (rm->doorPositions)
	{
		for (i = 0; i < roomSize; i++)
		{
			rm->doorPositions[i] = malloc(sizeof(int) * 2);
		}
	}

	for (y = 0; y < WINDOW_HEIGHT; y ++) {
		addPosition = 0;

		for (x = 0; x < WINDOW_WIDTH; x ++) {
			if (ROOM_MAP[x][y] == id) {
				xAvg += x;
				xAvgLen ++;
				addPosition = 1;
				rm->positionList[positionIndex][0] = x;
				rm->positionList[positionIndex][1] = y;

				positionIndex ++;
			}
		}

		if (addPosition) {
			yAvg += y;
			yAvgLen ++;
		}
	}

	rm->centerX = xAvg / xAvgLen;
	rm->centerY = yAvg / yAvgLen;
	
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

void addRoomDoorPosition(room *srcRoom, int x, int y) {
	srcRoom->doorPositions[srcRoom->numberOfDoorPositions][0] = x;
	srcRoom->doorPositions[srcRoom->numberOfDoorPositions][1] = y;
	
	srcRoom->numberOfDoorPositions ++;
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

	for (i = 0; i < srcRoom->numberOfConnectedRooms; i++) {
		if (srcRoom->connectedRooms[i] == dstRoom->id) {
			return 1;
		}
	}

	return 0;
}

int isRoomConnectedToId(room *srcRoom, int dstRoomId) {
	int i;

	for (i = 0; i < srcRoom->numberOfConnectedRooms; i++) {
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

	if (isLevelComplete() && player->itemLight) {
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
	
	for (i = 0; i < TCOD_random_get_int(RANDOM, 9, 12 + getRandomInt(5, 44)); i++) {
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

			/*if (ROOM_COUNT == ROOM_COUNT_MAX) {
				roomFlags = IS_TREASURE_ROOM;
			} else if (getRandomInt(0, 2)) {
				roomFlags = IS_TORCH_ROOM | IS_EXIT_ROOM;
			}*/

			createRoom(ROOM_COUNT, oLen, 0x0);

			printf("Found new room: %i (%i)\n", ROOM_COUNT, oLen);
		}
	}

	ROOM_COUNT_MAX = ROOM_COUNT;
}

int isLevelValid() {
	
	return 1;
	
	int i, ii, invalid, closedList[MAX_CONNECTED_ROOMS];
	int connectedRoomsIndex = 0;
	room *roomPtr = ROOMS;
	
	while (roomPtr) {
		for (i = 0; i < roomPtr->numberOfConnectedRooms; i++) {
			invalid = 0;
			
			for (ii = 0; ii < connectedRoomsIndex; ii++) {
				if (roomPtr->connectedRooms[i] == closedList[ii]) {
					invalid = 1;
					
					break;
				}
			}
			
			if (!invalid) {
				closedList[connectedRoomsIndex] = roomPtr->connectedRooms[i];
				connectedRoomsIndex ++; 
			}
		}
		
		roomPtr = roomPtr->next;
	}
	
	if (connectedRoomsIndex == MAX_ROOMS - 1) {
		return 1;
	}
	
	return 0;
}

void placeTunnels() {
	int x, y, x1, y1, w_x, w_y, mapUpdates, currentValue, neighborValue, lowestValue, index, lowestX, lowestY, invalid, randomRoomSize, dist;
	int numberOfFailedAttemptsToFindADestRoom, neighborCollision, banDoubleTunnels, srcRoomIndex, dstRoomIndex, startCount = 0, runCount = -1;
	int doorPlaced, destDoorPlaced, ownsTunnels;//, openRoomList[MAX_ROOMS], closedRoomList[MAX_ROOMS], openListCount, closedListCount, inClosedList, inOpenList, i, ii, id;
	room *srcRoom = NULL, *dstRoom = NULL;
	
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
			srcRoomIndex = getRandomInt(1, ROOM_COUNT_MAX);
			srcRoom = getRoomViaId(srcRoomIndex);
			numberOfFailedAttemptsToFindADestRoom = 0;

			while (dstRoomIndex == -1 || isRoomConnectedToId(srcRoom, dstRoomIndex)) {
				dstRoomIndex = getRandomInt(1, ROOM_COUNT_MAX);
				dstRoom = getRoomViaId(dstRoomIndex);

				if (numberOfFailedAttemptsToFindADestRoom < 5) {
					numberOfFailedAttemptsToFindADestRoom ++;
				} else {
					break;
				}
			}

			if (numberOfFailedAttemptsToFindADestRoom == 5) {
				continue;
			}
			
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

		doorPlaced = 0;
		destDoorPlaced = 0;
		
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

			w_x = lowestX;
			w_y = lowestY;

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

						if (!doorPlaced) {// && srcRoom->flags & IS_TREASURE_ROOM) {
							//createDoor(w_x + x1, w_y + y1);
							addRoomDoorPosition(srcRoom, w_x + x1, w_y + y1);

							doorPlaced = 1;
						}
					}
					
					TCOD_map_set_properties(LEVEL_MAP, w_x + x1, w_y + y1, 1, 1);
				}
			}

			if (!destDoorPlaced) {
				for (y1 = -1; y1 <= 1; y1++) {
					for (x1 = -1; x1 <= 1; x1++) {
						if (!x1 && !y1) {
							continue;
						}

						if (ROOM_MAP[w_x + x1][w_y + y1] == dstRoom->id) {
							//createDoor(w_x, w_y);
							addRoomDoorPosition(dstRoom, w_x, w_y);

							destDoorPlaced = 1;

							break;
						}
					}

					if (destDoorPlaced) {
						break;
					}
				}
			}
		}

		if (ROOM_MAP[w_x][w_y] == dstRoom->id) {
			printf("Connecting rooms: %i, %i\n", srcRoom->id, dstRoom->id);

			connectRooms(srcRoom, dstRoom);

			//if (isLevelValid()) {
			//	break;
			//}
			if (srcRoom->numberOfConnectedRooms == ROOM_COUNT_MAX) {
				break;
			}
		} else {
			printf("A possible bad path...?\n");
		}
	}
}

void placeItemChest() {

}

void generatePuzzles() {
	int spawnIndex, exitPlaced = 0, treasureRooms = 0;
	room *roomPtr = ROOMS;

	while (roomPtr) {
		if (!exitPlaced && roomPtr->size <= 80) {
			roomPtr->flags = roomPtr->flags | IS_EXIT_ROOM;
			spawnIndex = getRandomInt(0, roomPtr->size - 1);

			EXIT_LOCATION[0] = roomPtr->positionList[spawnIndex][0];
			EXIT_LOCATION[1] = roomPtr->positionList[spawnIndex][1];
			exitPlaced = 1;
		}

		if (roomPtr->size >= 45 && roomPtr->size <= 80) {
			roomPtr->flags = roomPtr->flags | IS_TORCH_ROOM;
		}
		
		printf("Conned: %i\n", roomPtr->numberOfConnectedRooms);

		if (roomPtr->numberOfConnectedRooms <= 2) {
			roomPtr->flags = roomPtr->flags | IS_TREASURE_ROOM;
			roomPtr->flags = roomPtr->flags | NEEDS_DOORS;

			treasureRooms ++;
		}

		roomPtr = roomPtr->next;
	}

	roomPtr = ROOMS;

	while (roomPtr) {
		if (roomPtr->flags & IS_TORCH_ROOM) {
			spawnIndex = getRandomInt(0, roomPtr->size - 1);

			createBonfireKeystone(roomPtr->positionList[spawnIndex][0], roomPtr->positionList[spawnIndex][1]);
		}

		if (roomPtr->flags & IS_TREASURE_ROOM) {
			spawnIndex = getRandomInt(0, roomPtr->size - 1);

			createTreasure(roomPtr->positionList[spawnIndex][0], roomPtr->positionList[spawnIndex][1]);
		}

		roomPtr = roomPtr->next;
	}

	placeItemChest();
}

void spawnEnemies() {
	int x, y, spawnIndex, maxNumberOfVoidWorms, numberOfVoidWorms = 0;
	room *roomPtr = ROOMS;

	if (LEVEL_NUMBER >= 2) {
		maxNumberOfVoidWorms = 2;
	} else {
		maxNumberOfVoidWorms = 1;
	}

	while (roomPtr) {
		spawnIndex = getRandomInt(0, roomPtr->size - 1);
		x = roomPtr->positionList[spawnIndex][0];
		y = roomPtr->positionList[spawnIndex][1];

		if (roomPtr->flags & IS_TREASURE_ROOM) {
			createBat(x, y);
		} else if (numberOfVoidWorms < maxNumberOfVoidWorms) {
			createVoidWorm(x, y);

			numberOfVoidWorms ++;
		} else if (roomPtr->flags & IS_EXIT_ROOM) {
		}

		roomPtr = roomPtr->next;
	}
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
		}

		itm = next;
	}
}

void activateDoors() {
	int i;
	room *roomPtr = ROOMS;
	item *itm = getItems();
	
	while (roomPtr) {
		if (roomPtr->flags & NEEDS_DOORS) {
			for (i = 0; i < roomPtr->numberOfDoorPositions; i++) {
				createDoor(roomPtr->doorPositions[i][0], roomPtr->doorPositions[i][1]);
			}
		}
		
		roomPtr = roomPtr->next;
	}

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
			r = 255;
			g = 255;
			b = 0;

			bMod = 170;
		} else if (roomPtr->flags & IS_TORCH_ROOM) {
			r = 140 - RED_SHIFT;
			g = 0;
			b = 140;

			rMod = 70;
			gMod = 70;
			bMod = 170;
		} else if (roomPtr->flags & IS_EXIT_ROOM) {
			r = 10;
			g = 10;
			b = 10;

			rMod = 250;
			gMod = 70;
			bMod = 70;
		} else if (roomPtr->numberOfConnectedRooms >= 3) {
			r = 205;
			g = 35;
			b = 30;
			rMod = 120;
			gMod = 30;
			bMod = 30;
		} else {
			r = 205;
			g = 25;
			b = 25;
		}

		for (i = 0; i < roomPtr->size; i ++) {
			x = roomPtr->positionList[i][0];
			y = roomPtr->positionList[i][1];

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

	smooth();
	findRooms();
	spawnEnemies();

	placeTunnels();
	generatePuzzles();
	//cleanUpDoors();
	activateDoors();

	//drawLights();
	drawDynamicLights();
	colorRooms();
	
	if (player) {
		roomPtr = ROOMS;

		while (roomPtr) {
			if (roomPtr->flags & IS_TREASURE_ROOM) {
				roomPtr = roomPtr->next;
			}

			if (!startingRoom || roomPtr->numberOfConnectedRooms > startingRoom->numberOfConnectedRooms) {
				startingRoom = roomPtr;
			}

			roomPtr = roomPtr->next;
		}
		player->x = startingRoom->centerX;
		player->y = startingRoom->centerY;
		player->itemLight->x = player->x;
		player->itemLight->y = player->y;
		player->vx = 1;
		
		spawnIndex = getRandomInt(0, startingRoom->size - 1);
		createKey(startingRoom->positionList[spawnIndex][0], startingRoom->positionList[spawnIndex][1]);
		
		spawnIndex = getRandomInt(0, startingRoom->size - 1);
		createRagdoll(startingRoom->positionList[spawnIndex][0], startingRoom->positionList[spawnIndex][1]);
		
		printf("Spawning at %i, %i\n", player->x, player->y);
	}

	if (player) {
		if (LEVEL_NUMBER == 1) {
			plantTorch(player);
			//createVoidWorm(player->x + 1, player->y + 1);
			//showMessage("%cSomething watches from the shadows...%c", 10);
		} else {
			spawnIndex = getRandomInt(0, startingRoom->size - 1);
			createBonfire(startingRoom->positionList[spawnIndex][0], startingRoom->positionList[spawnIndex][1]);
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
