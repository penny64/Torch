#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

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
#include "rooms.h"


TCOD_console_t LEVEL_CONSOLE;
TCOD_console_t LIGHT_CONSOLE;
TCOD_console_t SHADOW_CONSOLE;
TCOD_console_t FOG_CONSOLE;
TCOD_console_t SEEN_CONSOLE;
TCOD_map_t LEVEL_MAP;
TCOD_map_t LAVA_MAP;
TCOD_map_t TUNNEL_MAP;
TCOD_map_t TUNNEL_WALLS;
TCOD_noise_t FOG_NOISE;
room *STARTING_ROOM = NULL;
roomProto *PROTO_ROOMS[255];
int (*ROOM_MAP)[255];
int (*TUNNEL_ROOM_MAP)[255];
int (*DIJKSTRA_MAP)[255];
int (*CLOSED_MAP)[255];
float **EFFECTS_MAP;
float EXIT_WAVE_DIST;
int ROOM_COUNT, ROOM_COUNT_MAX, PROTO_ROOM_COUNT;
int EXIT_OPEN = 0;
int EXIT_IN_PROGRESS;
int START_LOCATION[2];
int EXIT_LOCATION[2];
int LEVEL_NUMBER;
int LEVEL_TYPE = LEVEL_KEYTORCH;
int (*openList)[WINDOW_WIDTH * WINDOW_HEIGHT];
int (*START_POSITIONS)[WINDOW_WIDTH * WINDOW_HEIGHT];


void levelSetup() {
	int i;

	LEVEL_CONSOLE = TCOD_console_new(WINDOW_WIDTH, WINDOW_HEIGHT);
	SHADOW_CONSOLE = TCOD_console_new(WINDOW_WIDTH, WINDOW_HEIGHT);
	FOG_CONSOLE = TCOD_console_new(WINDOW_WIDTH, WINDOW_HEIGHT);
	SEEN_CONSOLE = TCOD_console_new(WINDOW_WIDTH, WINDOW_HEIGHT);
	LIGHT_CONSOLE = TCOD_console_new(WINDOW_WIDTH, WINDOW_HEIGHT);
	LEVEL_MAP = TCOD_map_new(WINDOW_WIDTH, WINDOW_HEIGHT);
	LAVA_MAP = TCOD_map_new(WINDOW_WIDTH, WINDOW_HEIGHT);
	TUNNEL_MAP = TCOD_map_new(WINDOW_WIDTH, WINDOW_HEIGHT);
	TUNNEL_WALLS = TCOD_map_new(WINDOW_WIDTH, WINDOW_HEIGHT);
	FOG_NOISE = TCOD_noise_new(2, TCOD_NOISE_DEFAULT_HURST, TCOD_NOISE_DEFAULT_LACUNARITY, NULL);

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
	openList = calloc(1, sizeof(double[WINDOW_WIDTH * WINDOW_HEIGHT][WINDOW_WIDTH * WINDOW_HEIGHT]));
	DIJKSTRA_MAP = malloc(sizeof(double[255][255]));
	START_POSITIONS = malloc(sizeof(double[WINDOW_WIDTH * WINDOW_HEIGHT][WINDOW_WIDTH * WINDOW_HEIGHT]));
	EFFECTS_MAP = malloc(WINDOW_HEIGHT * sizeof(float *));
	LEVEL_NUMBER = 1;

	for (i = 0; i < WINDOW_HEIGHT; i ++) {
		EFFECTS_MAP[i] = malloc(WINDOW_WIDTH * sizeof(float));
	}
	
	startLights();
}

void levelShutdown() {
	printf("Shutting down level...\n");
	
	deleteAllRooms();
	free(ROOM_MAP);
	free(TUNNEL_ROOM_MAP);
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
	TCOD_map_delete(LAVA_MAP);
	TCOD_map_delete(TUNNEL_MAP);
	TCOD_map_delete(TUNNEL_WALLS);
	TCOD_noise_delete(FOG_NOISE);
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

TCOD_map_t getTunnelMap() {
	return TUNNEL_WALLS;
}

TCOD_map_t getSeenConsole() {
	return SEEN_CONSOLE;
}

TCOD_map_t copyLevelMap() {
	TCOD_map_t newMap = TCOD_map_new(WINDOW_WIDTH, WINDOW_HEIGHT);
	TCOD_map_copy(LEVEL_MAP, newMap);
	
	return newMap;
}

int *getRoomMap() {
	return ROOM_MAP[0];
}

float **getEffectsMap() {
	return EFFECTS_MAP;
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

	showMessage(20, "You hear a low rumble.", NULL);
}

void exitLevel() {
	EXIT_IN_PROGRESS = 1;
	LEVEL_NUMBER ++;

	showMessage(20, "You step down...", NULL);
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

int checkForLevelCompletion() {
	if (LEVEL_TYPE == LEVEL_KEYTORCH) {
		if (getTotalNumberOfKeytorches() > 0 && !isLevelComplete() && getNumberOfLitKeytorches() == getTotalNumberOfKeytorches()) {
			return 1;
		}
	} else if (LEVEL_TYPE == LEVEL_PLAIN) {
		return 1;
	}

	return 0;
}

int levelLogic() {
	character *player = getPlayer();

	if (!isLevelComplete() && checkForLevelCompletion()) {
		completeLevel();
	}

	if (isLevelComplete() && player->itemLight) {
		EXIT_WAVE_DIST = clipFloat(EXIT_WAVE_DIST + .5f, 0, 255);

		if (!player->itemLight->sizeMod && isScreenFadedOut()) {
			generateLevel();

			return 1;
		}
	}

	return 0;
}

void generateKeys() {
	int i, ii, invalid, spawnPosition[2], openList[MAX_CONNECTED_ROOMS], closedList[MAX_CONNECTED_ROOMS];
	int openListIndex = 0, connectedRoomsIndex = 0;
	room *lastRoomPtr = NULL, *roomPtr = getRoomWithFlags(IS_START_ROOM);
	
	if (!roomPtr) {
		printf("*FATAL* No start room found.\n");
		
		return;
	}

	openList[openListIndex] = roomPtr->id;
	openListIndex ++;
	
	while (openListIndex) {
		openListIndex --;
		lastRoomPtr = roomPtr;
		roomPtr = getRoomViaId(openList[openListIndex]);
		closedList[connectedRoomsIndex] = roomPtr->id;
		connectedRoomsIndex ++;
		
		if (roomPtr->flags & NEEDS_DOORS) {
			getNewSpawnPosition(lastRoomPtr, spawnPosition);
			spawnItemWithRarity(spawnPosition[0], spawnPosition[1], IS_KEY, RARITY_KEY, RARITY_KEY);
		}

		for (i = 0; i < roomPtr->numberOfNeighborRooms; i++) {
			invalid = 0;
			
			for (ii = 0; ii < connectedRoomsIndex; ii++) {
				if (roomPtr->neighborRoomIds[i] == closedList[ii]) {
					invalid = 1;
					
					break;
				}
			}

			if (!invalid) {
				for (ii = 0; ii < openListIndex; ii++) {
					if (roomPtr->neighborRoomIds[i] == openList[ii]) {
						invalid = 1;

						break;
					}
				}
			}

			if (!invalid) {
				openList[openListIndex] = roomPtr->neighborRoomIds[i];
				openListIndex ++;
			}
		}
	}
}

void generatePuzzles() {
	int i, numberOfTorchRooms = 0, numberOfTreasureRooms = 0, numberOfFurances = 0;
	room *neighborRoomPtr, *roomPtr = getRooms();

	while (roomPtr) {
		printf("Conned: %i, size: %i\n", roomPtr->numberOfNeighborRooms, roomPtr->size);

		if (roomPtr->flags & IS_START_ROOM) {
			roomPtr = roomPtr->next;

			continue;
		}

		if (roomPtr->flags & IS_SPECIAL_ROOM) {
			roomPtr->flags |= IS_TREASURE_ROOM;
			roomPtr->flags |= NEEDS_DOORS;

			roomPtr = roomPtr->next;

			continue;
		}

		if (!(roomPtr->flags & IS_MAIN_PATH) && numberOfTreasureRooms < 2 && roomPtr->numberOfNeighborRooms <= 2 && roomPtr->size < 80) {
			roomPtr->flags |= IS_TREASURE_ROOM;
			roomPtr->flags |= NEEDS_DOORS;

			numberOfTreasureRooms++;
		} else if (LEVEL_TYPE == LEVEL_KEYTORCH && roomPtr->size >= 45 && roomPtr->size <= 65 && numberOfTorchRooms <= 3) {
			roomPtr->flags |= IS_TORCH_ROOM;
		} else if (roomPtr->size >= 80 && roomPtr->size <= 90) {
			if (roomPtr->numberOfNeighborRooms == 2) {
				roomPtr->flags |= IS_LAVA_ROOM;
			} else if (!(roomPtr->flags & IS_MAIN_PATH)) {
				roomPtr->flags |= NEEDS_DOORS;
			}
		}
		
		if (roomPtr->numberOfNeighborRooms == 1) {
			roomPtr->flags |= IS_RARE_SPAWN;
		}

		roomPtr = roomPtr->next;
	}

	roomPtr = getRooms();

	while (roomPtr && !numberOfFurances) {
		for (i = 0; i < roomPtr->numberOfNeighborRooms; i ++) {
			neighborRoomPtr = getRoomViaId(roomPtr->neighborRoomIds[i]);

			if (neighborRoomPtr->flags & IS_LAVA_ROOM) {
				roomPtr->flags |= IS_FURNACE_ROOM;
				numberOfFurances ++;

				break;
			}
		}

		roomPtr = roomPtr->next;
	}
}

void placeItems() {
	int x, y, i, lavaWalkerX, lavaWalkerY, doorEnterIndex, doorExitIndex, placedAllSeeingEye = 0;
	int spawnPosition[2], doorEnter[2], doorExit[2];
	TCOD_dijkstra_t lavaWalker = TCOD_dijkstra_new(LEVEL_MAP, 0.0f);
	room *roomPtr = getRooms();
	light *lghtPtr;

	while (roomPtr) {
		if (roomPtr->flags & IS_TORCH_ROOM) {
			createAndPlaceItemInRoom(roomPtr, &createBonfireKeystone);
		}

		if (!placedAllSeeingEye) {
			if (roomPtr->size <= 15 && getRandomFloat(0, 1) >= .6) {
				createAndPlaceItemInRoom(roomPtr, &createAllSeeingEye);

				placedAllSeeingEye = 1;
			}
		}

		if (roomPtr->flags & IS_TREASURE_ROOM) {
			getNewSpawnPosition(roomPtr, spawnPosition);
			createTreasure(spawnPosition[0], spawnPosition[1]);

			getNewSpawnPosition(roomPtr, spawnPosition);
			spawnItemWithRarity(spawnPosition[0], spawnPosition[1], IS_WEAPON, RARITY_LOW, RARITY_LOW);
		}

		if (roomPtr->flags & IS_RARE_SPAWN) {
			getNewSpawnPosition(roomPtr, spawnPosition);

			spawnItemWithRarity(spawnPosition[0], spawnPosition[1], IS_ARMOR, RARITY_MEDIUM, RARITY_HIGH);
		}

		if (roomPtr->flags & IS_LAVA_ROOM) {
			for (i = 0; i < roomPtr->size; i++) {
				TCOD_map_set_properties(LAVA_MAP, roomPtr->positionList[i][0], roomPtr->positionList[i][1], 1, 1);
			}

			for (doorEnterIndex = 0; doorEnterIndex < roomPtr->numberOfDoorPositions; doorEnterIndex ++) {
				doorEnter[0] = roomPtr->doorPositions[doorEnterIndex][0];
				doorEnter[1] = roomPtr->doorPositions[doorEnterIndex][1];

				TCOD_dijkstra_compute(lavaWalker, doorEnter[0], doorEnter[1]);

				for (doorExitIndex = doorEnterIndex + 1; doorExitIndex < roomPtr->numberOfDoorPositions; doorExitIndex ++) {
					doorExit[0] = roomPtr->doorPositions[doorExitIndex][0];
					doorExit[1] = roomPtr->doorPositions[doorExitIndex][1];

					TCOD_dijkstra_path_set(lavaWalker, doorExit[0], doorExit[1]);

					printf("From: %i, %i, to: %i, %i\n", doorEnter[0], doorEnter[1], doorExit[0], doorExit[1]);

					while (TCOD_dijkstra_path_walk(lavaWalker, &lavaWalkerX, &lavaWalkerY)) {
						TCOD_map_set_properties(LAVA_MAP, lavaWalkerX, lavaWalkerY, 0, 0);
					}
				}
			}

			for (i = 0; i < roomPtr->size; i++) {
				x = roomPtr->positionList[i][0];
				y = roomPtr->positionList[i][1];

				if (!TCOD_map_is_walkable(LAVA_MAP, x, y)) {
					claimSpawnPositionInRoom(roomPtr, x, y);
				}
			}

			for (i = 0; i < roomPtr->size; i ++) {
				x = roomPtr->positionList[i][0];
				y = roomPtr->positionList[i][1];

				if (!TCOD_map_is_walkable(LAVA_MAP, x, y)) {
					continue;
				}

				if (getRandomFloat(0, 1) >= .85) {
					lghtPtr = createDynamicLight(x, y, NULL);

					lghtPtr->size = getRandomInt(2, 3);
					lghtPtr->r_tint = 20;
					lghtPtr->g_tint = 20;
					lghtPtr->b_tint = 20;
					lghtPtr->brightness = .35;
					lghtPtr->fuel = 9999;
					lghtPtr->fuelMax = 9999;
					lghtPtr->flickerRate = .08;
				}
			}
		}

		roomPtr = roomPtr->next;
	}
}

float roomPositionCost(int xFrom, int yFrom, int xTo, int yTo, void *user_data) {
	return 1.f;
}

void decorateRooms() {
	int i, ii, invalid, x, y, wX, wY, relX, relY, nx, ny, x1, y1, isNextToWall;
	room *roomPtr = getRooms();
	TCOD_map_t roomMap = NULL;
	TCOD_dijkstra_t dijkstraPath = NULL;

	while (roomPtr) {
		roomMap = TCOD_map_new(WINDOW_WIDTH, WINDOW_HEIGHT);
		TCOD_map_copy(LEVEL_MAP, TUNNEL_MAP);

		//#TODO: Make all positions in rooms walkable

		for (i = 0; i < roomPtr->size; i ++) {
			TCOD_map_set_properties(roomMap, roomPtr->positionList[i][0], roomPtr->positionList[i][1], 1, 1);
		}

		//dijkstraPath = TCOD_dijkstra_new_using_function(roomPtr->width, roomPtr->height, roomPositionCost, roomPtr, 0);
		dijkstraPath = TCOD_dijkstra_new(roomMap, 0);

		if ((roomPtr->flags & IS_TORCH_ROOM) || (roomPtr->flags & IS_FURNACE_ROOM)) {
			for (i = 0; i < roomPtr->size; i ++) {
				x = roomPtr->positionList[i][0];
				y = roomPtr->positionList[i][1];

				isNextToWall = 0;
				invalid = 0;

				for (x1 = -1; x1 <= 1; x1 ++) {
					for (y1 = -1; y1 <= 1; y1 ++) {
						//if (TCOD_map_is_walkable(TUNNEL_MAP, x + x1, y + y1)) {
						//	invalid = 1;

						//	break;
						//}

						if (x + x1 < 0 || x + x1 >= WINDOW_WIDTH || y + y1 < 0 || y + y1 >= WINDOW_HEIGHT) {
							continue;
						}

						if ((y1 == -1 && x1 == 1) || (y1 == -1 && x1 == -1) || (y1 == 1 && x1 == 1) || (y1 == 1 && x1 == -1)) {
							continue;
						}

						nx = x + x1;
						ny = y + y1;

						for (ii = 0; ii < roomPtr->numberOfDoorPositions; ii ++) {
							if (nx == roomPtr->doorPositions[ii][0] && ny == roomPtr->doorPositions[ii][1]) {
								ii = -1;

								break;
							}
						}

						//if (!isPositionSpawnable(roomPtr, nx, ny)) {
						//	continue;
						//}

						if (ii == -1) {
							invalid = 1;

							break;
						}

						if (!isPositionWalkable(nx, ny)) {
							isNextToWall = 1;
						}
					}

					if (invalid) {
						isNextToWall = 0;

						break;
					}
				}

				if (isNextToWall) {
					claimSpawnPositionInRoom(roomPtr, x, y);

					if (roomPtr->flags & IS_TORCH_ROOM) {
						createWoodWall(x, y);
					} else if (roomPtr->flags & IS_FURNACE_ROOM) {
						createMetalWall(x, y);
					}
				}
			}
		}

		if (roomPtr->flags & IS_TREASURE_ROOM) {
			for (y = roomPtr->y; y < roomPtr->y + roomPtr->height; y++) {
				for (x = roomPtr->x; x < roomPtr->x + roomPtr->width; x++) {
					relX = x - roomPtr->x;
					relY = y - roomPtr->y;

					if (relX <= 1 || relY <= 1 || relX >= roomPtr->width - 2 || relY >= roomPtr->height - 2) {
						continue;
					}

					if ((relX % (roomPtr->width / 2)) && !(relY % 2)) {
						createMetalWall(x, y);

						claimSpawnPositionInRoom(roomPtr, x, y);
					}
				}
			}
		} else {
			if (!(roomPtr->flags & IS_MAIN_PATH) && roomPtr->numberOfDoorPositions > 1) {
				for (i = 1; i < roomPtr->numberOfDoorPositions; i++) {
					TCOD_dijkstra_compute(dijkstraPath, roomPtr->doorPositions[i][0], roomPtr->doorPositions[i][1]);
					TCOD_dijkstra_path_set(dijkstraPath, roomPtr->doorPositions[i - 1][0], roomPtr->doorPositions[i - 1][1]);

					printf("%f\n", TCOD_dijkstra_get_distance(dijkstraPath, roomPtr->doorPositions[i - 1][0], roomPtr->doorPositions[i - 1][1]));

					while (TCOD_dijkstra_path_walk(dijkstraPath, &wX, &wY)) {
						//TCOD_map_set_properties(roomMap, wX, wY, 0, 0);

						printf("%i, %i\n", wX, wY);
					}
				}

				for (y = roomPtr->y; y < roomPtr->y + roomPtr->height; y++) {
					for (x = roomPtr->x; x < roomPtr->x + roomPtr->width; x++) {
						relX = x - roomPtr->x;
						relY = y - roomPtr->y;

						if (TCOD_map_is_walkable(roomMap, relX, relY)) {
							createWoodWall(x, y);
						}
					}
				}
			}
		}

		TCOD_map_delete(roomMap);
		TCOD_dijkstra_delete(dijkstraPath);

		roomMap = NULL;
		dijkstraPath = NULL;
		roomPtr = roomPtr->next;
	}
}

void spawnEnemies() {
	int spawnPosition[2], maxNumberOfVoidWorms, numberOfRagdolls = 0, numberOfVoidWorms = 0;
	room *roomPtr = getRooms();

	if (LEVEL_NUMBER >= 2) {
		maxNumberOfVoidWorms = 2;
	} else {
		maxNumberOfVoidWorms = 1;
	}

	while (roomPtr) {
		if (roomPtr->flags & IS_TREASURE_ROOM) {
			getNewSpawnPosition(roomPtr, spawnPosition);

			createBat(spawnPosition[0], spawnPosition[1]);
		} else if (numberOfVoidWorms < maxNumberOfVoidWorms) {
			getNewSpawnPosition(roomPtr, spawnPosition);

			createVoidWorm(spawnPosition[0], spawnPosition[1]);

			numberOfVoidWorms ++;
		} else if (roomPtr->flags & IS_EXIT_ROOM) {
		} else {
			if (numberOfRagdolls < clip(LEVEL_NUMBER + 3, 0, 8) && !getRandomInt(0, 3 + numberOfRagdolls)) {
				getNewSpawnPosition(roomPtr, spawnPosition);

				createRagdoll(spawnPosition[0], spawnPosition[1]);
				numberOfRagdolls ++;
			}
		}

		if (roomPtr->flags & IS_FURNACE_ROOM && getRandomInt(0, 3)) {
			getNewSpawnPosition(roomPtr, spawnPosition);

			createRagdoll(spawnPosition[0], spawnPosition[1]);
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
	room *roomPtr = getRooms();
	item *itm = getItems();
	
	while (roomPtr) {
		if (roomPtr->flags & NEEDS_DOORS) {
			printf("Needs doors: %i\n", roomPtr->numberOfDoorPositions);

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

		if (itm->itemFlags & IS_SOLID) {
			enableSolid(itm);
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
	room *roomPtr = getRooms();

	while (roomPtr) {
		//printf("FLAGS: %i, %i\n", roomPtr->flags, 0x01 << 1);

		rMod = 0;
		gMod = 0;
		bMod = 0;

		if (roomPtr->flags & IS_TREASURE_ROOM) { //CHECKED
			printf("Treasure\n");
			r = 120;
			g = 101;
			b = 23;
		} else if (roomPtr->flags & IS_LAVA_ROOM) { //CHECKED
			printf("Lava\n");
			r = 200;
			g = 200;
			b = 200;
		} else if (roomPtr->flags & IS_TORCH_ROOM) { //CHECKED
			printf("Torch\n");
			r = 140 - RED_SHIFT;
			g = 0;
			b = 140;

			rMod = 70;
			gMod = 70;
			bMod = 170;
		} else if (roomPtr->flags & IS_EXIT_ROOM) { //CHECKED
			r = 145;
			g = 145;
			b = 145;

			rMod = 10;
			gMod = 60;
			bMod = 80;
		} else if (roomPtr->numberOfNeighborRooms >= 3) {
			if (roomPtr->size <= 30) { //CHECKED
				r = 0 - RED_SHIFT;
				g = 155;
				b = 155;
			} else { //CHECKED
				r = 135 - RED_SHIFT;
				g = 135;
				b = 135;
			}

			rMod = 60;
			gMod = 60;
			bMod = 60;
		} else { //CHCKED
			r = 120;
			g = 0;
			b = 120;

			rMod = 45;
			gMod = 15;
			bMod = 45;
		}
		
		for (i = 0; i < roomPtr->size; i ++) {
			x = roomPtr->positionList[i][0];
			y = roomPtr->positionList[i][1];

			if (TCOD_map_is_walkable(LAVA_MAP, x, y)) {
				drawCharBackEx(LEVEL_CONSOLE, x, y, TCOD_color_RGB(255 - getRandomInt(0, 75), 0, 0), TCOD_BKGND_SET);
			} else {
				drawCharBackEx(LEVEL_CONSOLE, x, y, TCOD_color_RGB(clip(r + RED_SHIFT + getRandomInt(0, rMod), 0, 255), clip(g + getRandomInt(0, gMod), 0, 255), clip(b + getRandomInt(0, bMod), 0, 255)), TCOD_BKGND_SET);
			}
		}

		/*for (y = 2; y < WINDOW_HEIGHT - 2; y ++) {
			for (x = 2; x < WINDOW_WIDTH - 2; x ++) {
				if (!TCOD_map_is_walkable(TUNNEL_MAP, x, y)) {
					colorMod = getRandomInt(0, 15);

					drawCharBackEx(LEVEL_CONSOLE, x, y, TCOD_color_RGB(200 + RED_SHIFT - colorMod, 200 - colorMod, 200 - colorMod), TCOD_BKGND_SET);
				}
			}
		}*/

		roomPtr = roomPtr->next;
	}
}

void colorItems() {
	item *itemPtr = getItems();

	while (itemPtr) {
		if (itemPtr->itemFlags & IS_KEYTORCH) {
			drawCharBackEx(getLevelConsole(), itemPtr->x, itemPtr->y, TCOD_color_RGB(50, 50, 50), TCOD_BKGND_SET);
		}

		itemPtr = itemPtr->next;
	}
}

void placeGrass() {
	room *roomPtr = getRooms();
	int x, y, i, colorMod, tileChar;
	float tileRange, fogValue, fogPoint[2];
	TCOD_noise_t fog = getFogNoise();

	while (roomPtr) {
		for (i = 0; i < roomPtr->size; i++) {
			x = roomPtr->positionList[i][0];
			y = roomPtr->positionList[i][1];

			if (TCOD_map_is_walkable(LAVA_MAP, x, y)) {
				continue;
			}

			fogPoint[0] = (float) x / WINDOW_WIDTH;
			fogPoint[1] = (float) y / WINDOW_HEIGHT;

			fogValue = TCOD_noise_get_fbm_ex(fog, fogPoint, 32.0f, TCOD_NOISE_PERLIN) + .4f;

			if (fogValue <= .5) {
				tileRange = fogValue / .5;
				colorMod = getRandomInt(25, 65);

				if (tileRange >= .75) {
					tileChar = (int) '.';
				} else if (tileRange >= .5) {
					tileChar = (int) ',';
				} else if (tileRange >= .25) {
					tileChar = (int) ':';
				} else {
					tileChar = 141;
				}

				setCharEx(LEVEL_CONSOLE, x, y, tileChar, TCOD_color_RGB(0, 205 - (colorMod * 1.5), 42));
				drawCharBackEx(LEVEL_CONSOLE, x, y, TCOD_color_RGB(0, 105 - colorMod, 12), TCOD_BKGND_ALPHA(.7));
			}
		}

		roomPtr = roomPtr->next;
	}
}

void pickRoomType() {
	LEVEL_TYPE = getRandomInt(0, 1);
}

void resetLevel() {
	TCOD_console_t dynamicLightConsole = getDynamicLightConsole();

	EXIT_OPEN = 0;
	EXIT_WAVE_DIST = 0;

	TCOD_map_clear(LEVEL_MAP, 0, 0);
	TCOD_map_clear(LAVA_MAP, 0, 0);
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
}

void paintLevel() {
	int x, y;
	float fogValue, p[2];
	TCOD_noise_t fog = getFogNoise();

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

			EFFECTS_MAP[x][y] = getRandomFloat(.65, .8);

			if (!TCOD_map_is_walkable(LEVEL_MAP, x, y)) {
				if (TCOD_map_is_walkable(TUNNEL_WALLS, x, y)) {
					//drawChar(LEVEL_CONSOLE, x, y, 128, TCOD_color_RGB(125, 16, 16), TCOD_color_RGB(115, 6, 6));
					//drawCharBackEx(LEVEL_CONSOLE, x, y, TCOD_color_RGB(125, 16, 16), TCOD_BKGND_SET);
					drawChar(LEVEL_CONSOLE, x, y, 176, TCOD_color_RGB(200, 36, 36), TCOD_color_RGB(105, 26, 105));
				} else {
					drawChar(LEVEL_CONSOLE, x, y, 177 + getRandomInt(0, 1), TCOD_color_RGB(175, 36, 36), TCOD_color_RGB(105, 26, 26));
					//drawCharBackEx(LEVEL_CONSOLE, x, y, TCOD_color_RGB(95, 8, 8), TCOD_BKGND_SET);
				}
			} else if (!TCOD_map_is_walkable(LAVA_MAP, x, y)) {
				//colorMod = (int)(fogValue * 120);

				//if (!TCOD_random_get_int(RANDOM, 0, 4)) {
				//	setCharEx(LEVEL_CONSOLE, x, y, ',' + TCOD_random_get_int(RANDOM, 0, 4), TCOD_color_RGB(155 - colorMod, 290 - colorMod, 190 - colorMod));
				//}

				//drawCharBackEx(FOG_CONSOLE, x, y, TCOD_color_RGB(135 - colorMod, 120 - colorMod, 120 - colorMod), TCOD_BKGND_ALPHA(1));
			}
		}
	}
}

void buildDungeon() {
	int i, ii, invalidRoom, horizSplit, nRoomCount, mapUpdated = 1, minRoomSize = 190, bannedRoomCount = 0;
	int MAX_ROOMS_TEMP = 60;
	roomProto *roomWalker;
	roomProto *rootRoom = createProtoRoom(2, 2, WINDOW_WIDTH - 2, WINDOW_HEIGHT - 2, NULL);
	roomProto *bannedRooms[MAX_ROOMS_TEMP];

	PROTO_ROOM_COUNT = 0;
	PROTO_ROOMS[0] = rootRoom;
	PROTO_ROOM_COUNT++;

	while (mapUpdated) {
		mapUpdated = 0;
		nRoomCount = PROTO_ROOM_COUNT;

		for (i = 0; i < nRoomCount; i ++) {
			invalidRoom = 0;
			roomWalker = PROTO_ROOMS[i];

			if (roomWalker->size <= minRoomSize * 1.3 && !getRandomInt(0, 15)) {
				bannedRooms[bannedRoomCount] = roomWalker;
				bannedRoomCount ++;

				break;
			}

			for (ii = 0; ii < bannedRoomCount; ii ++) {
				if (roomWalker == bannedRooms[ii]) {
					invalidRoom = 1;

					break;
				}
			}

			if (invalidRoom) {
				continue;
			}

			if (roomWalker->size <= minRoomSize || roomWalker->width <= 8 || roomWalker->height <= 8) {
				continue;
			}

			//if (roomWalker->width <= 13 && roomWalker->height <= 13) {
			//	continue;
			//}

			if (roomWalker->width > roomWalker->height) {
				horizSplit = 0;
			} else if (roomWalker->width < roomWalker->height) {
				horizSplit = 1;
			} else {
				horizSplit = getRandomInt(0, 1);
			}

			roomProto *childRoom = splitProtoRoom(roomWalker, horizSplit);

			PROTO_ROOMS[PROTO_ROOM_COUNT] = childRoom;
			PROTO_ROOM_COUNT++;
			mapUpdated = 1;

			assert(PROTO_ROOM_COUNT < MAX_ROOMS_TEMP);
		}
	}

	printf("Total room count: %i\n", PROTO_ROOM_COUNT);
}

float getPositionCost(int xFrom, int yFrom, int xTo, int yTo, void *user_data) {
	roomProto *roomWalker;
	int i, fromInRoom, toInRoom;
	float fromCost = 999, toCost = 999;

	for (i = 0; i < PROTO_ROOM_COUNT; i ++) {
		roomWalker = PROTO_ROOMS[i];

		fromInRoom = (xFrom >= roomWalker->x &&
				yFrom >= roomWalker->y &&
				xFrom < roomWalker->x + roomWalker->width &&
				yFrom < roomWalker->y + roomWalker->height);
		toInRoom = (xTo >= roomWalker->x &&
					  yTo >= roomWalker->y &&
					  xTo < roomWalker->x + roomWalker->width &&
					  yTo < roomWalker->y + roomWalker->height);

		if (fromInRoom) {
			fromCost = roomWalker->cost * roomWalker->timesSplit;

			if (roomWalker->build) {
				fromCost *= 2;
			}
		}

		if (toInRoom) {
			toCost = roomWalker->cost * roomWalker->timesSplit;

			if (roomWalker->build) {
				fromCost *= 2;
			}
		}
	}

	return fromCost + toCost;
}

void designDungeon() {
	int i, x, y, positionIndex, wX, wY, startRoomX, startRoomY, exitRoomX, exitRoomY, distanceToStart, distanceToExit, inRoom, specialRoomScore = 0, endRoomScore = 0;
	roomProto *roomWalker, *specialRoom = NULL, *startRoom = NULL, *endRoom = NULL;
	TCOD_path_t pathfinder;

	pathfinder = TCOD_path_new_using_function(WINDOW_WIDTH, WINDOW_HEIGHT, getPositionCost, NULL, 0);

	//Find start room
	for (i = 0; i < PROTO_ROOM_COUNT; i ++) {
		roomWalker = PROTO_ROOMS[i];

		if (!startRoom || !getRandomInt(0, 15)) {
			startRoom = roomWalker;
		}
	}

	startRoomX = startRoom->x + (startRoom->width / 2);
	startRoomY = startRoom->y + (startRoom->height / 2);

	//End room
	for (i = 0; i < PROTO_ROOM_COUNT; i ++) {
		roomWalker = PROTO_ROOMS[i];

		distanceToStart = distance(roomWalker->x + (roomWalker->width / 2),
								   roomWalker->y + (roomWalker->height / 2),
								   startRoomX,
								   startRoomY);

		if (distanceToStart > endRoomScore) {
			endRoomScore = distanceToStart;
			endRoom = roomWalker;
		}
	}

	//Cost assignment
	for (i = 0; i < PROTO_ROOM_COUNT; i ++) {
		roomWalker = PROTO_ROOMS[i];

		if (roomWalker == startRoom || roomWalker == endRoom) {
			continue;
		}

		roomWalker->cost = getRandomFloat(4.5f, 5.5f);
	}

	startRoom->cost = 1.;
	startRoom->flags |= IS_PROTO_START;

	endRoom->cost = 1.;
	endRoom->flags |= IS_PROTO_EXIT;

	exitRoomX = endRoom->x + (endRoom->width / 2);
	exitRoomY = endRoom->y + (endRoom->height / 2);

	//Find start and end rooms
	TCOD_path_compute(pathfinder,
					  startRoomX,
					  startRoomY,
					  exitRoomX,
					  exitRoomY);

	//FIRST WALKTHROUGH: THE DIRECT PATH
	while (TCOD_path_walk(pathfinder, &wX, &wY, 0)) {
		for (i = 0; i < PROTO_ROOM_COUNT; i ++) {
			roomWalker = PROTO_ROOMS[i];

			if (roomWalker->build) {
				continue;
			}

			inRoom = (wX >= roomWalker->x &&
					wY >= roomWalker->y &&
					wX < roomWalker->x + roomWalker->width &&
					wY < roomWalker->y + roomWalker->height);

			if (inRoom) {
				roomWalker->build = 1;
				roomWalker->flags |= IS_PROTO_MAIN_PATH;

				continue;
			}
		}
	}

	//SECOND WALKTHROUGH: THE HARD WAY
	TCOD_path_compute(pathfinder,
					  startRoomX,
					  startRoomY,
					  exitRoomX,
					  exitRoomY);

	while (TCOD_path_walk(pathfinder, &wX, &wY, 0)) {
		for (i = 0; i < PROTO_ROOM_COUNT; i ++) {
			roomWalker = PROTO_ROOMS[i];

			inRoom = (wX >= roomWalker->x &&
					  wY >= roomWalker->y &&
					  wX < roomWalker->x + roomWalker->width &&
					  wY < roomWalker->y + roomWalker->height);

			if (inRoom) {
				roomWalker->build = 1;

				continue;
			}
		}
	}

	for (i = 0; i < PROTO_ROOM_COUNT; i ++) {
		roomWalker = PROTO_ROOMS[i];

		inRoom = (wX >= roomWalker->x &&
				  wY >= roomWalker->y &&
				  wX < roomWalker->x + roomWalker->width &&
				  wY < roomWalker->y + roomWalker->height);

		if (inRoom) {
			roomWalker->build = 1;

			continue;
		}
	}

	for (i = 0; i < PROTO_ROOM_COUNT; i ++) {
		roomWalker = PROTO_ROOMS[i];

		if (roomWalker->build) {
			continue;
		}

		distanceToStart = distance(roomWalker->x + (roomWalker->width / 2),
								   roomWalker->y + (roomWalker->height / 2),
								   startRoomX,
								   startRoomY);
		distanceToExit = distance(roomWalker->x + (roomWalker->width / 2),
								  roomWalker->y + (roomWalker->height / 2),
								  exitRoomX,
								  exitRoomY);

		if ((distanceToStart + distanceToExit) > specialRoomScore) {
			specialRoomScore = distanceToStart + distanceToExit;
			specialRoom = roomWalker;
		}
	}

	//THIRD WALKTHROUGH: THE DANGEROUS WAY
	TCOD_path_compute(pathfinder,
					  startRoomX,
					  startRoomY,
					  specialRoom->x + (specialRoom->width / 2),
					  specialRoom->y + (specialRoom->height / 2));

	while (TCOD_path_walk(pathfinder, &wX, &wY, 0)) {
		for (i = 0; i < PROTO_ROOM_COUNT; i ++) {
			roomWalker = PROTO_ROOMS[i];

			inRoom = (wX >= roomWalker->x &&
					  wY >= roomWalker->y &&
					  wX < roomWalker->x + roomWalker->width &&
					  wY < roomWalker->y + roomWalker->height);

			if (inRoom) {
				roomWalker->build = 1;

				continue;
			}
		}
	}

	//Place rooms
	for (i = 0; i < PROTO_ROOM_COUNT; i ++) {
		if (!PROTO_ROOMS[i]->build) {
			continue;
		}

		room *rm = createRoom(PROTO_ROOMS[i], 0x0);

		if (PROTO_ROOMS[i]->flags & IS_PROTO_START) {
			rm->flags |= IS_START_ROOM;

			getNewSpawnPosition(rm, START_LOCATION);

			STARTING_ROOM = rm;
		}

		if (PROTO_ROOMS[i]->flags & IS_PROTO_EXIT) {
			rm->flags |= IS_EXIT_ROOM;

			getNewSpawnPosition(rm, EXIT_LOCATION);
		}

		if (PROTO_ROOMS[i]->flags & IS_PROTO_MAIN_PATH) {
			rm->flags |= IS_MAIN_PATH;
		}

		if (PROTO_ROOMS[i]->flags & IS_PROTO_SPECIAL_ROOM) {
			rm->flags |= IS_SPECIAL_ROOM;
		}

		for (positionIndex = 0; positionIndex < rm->size; positionIndex ++) {
			x = rm->positionList[positionIndex][0];
			y = rm->positionList[positionIndex][1];

			drawCharBackEx(LEVEL_CONSOLE, x, y, TCOD_color_RGB(255, 30, 255), TCOD_BKGND_SET);
			TCOD_map_set_properties(LEVEL_MAP, x, y, 1, 1);
		}
	}
}

void connectNeighbors() {
	int i, tempDistance, cloestRoomDistance, hasNeighborOfNeighbor;
	room *nearestChildPtr, *tempNeighbor, *childPtr, *parentPtr = getRooms();

	while (parentPtr) {
		childPtr = getRooms();
		nearestChildPtr = NULL;
		cloestRoomDistance = WINDOW_WIDTH * WINDOW_HEIGHT;

		while (childPtr) {
			hasNeighborOfNeighbor = 0;

			if (parentPtr == childPtr || isNeighborWith(parentPtr, childPtr)) {
				//for (i = 0; i < )
				childPtr = childPtr->next;

				continue;
			}

			for (i = 0; i < parentPtr->numberOfNeighborRooms; i ++) {
				tempNeighbor = getRoomViaId(parentPtr->neighborRoomIds[i]);

				if (isNeighborWith(tempNeighbor, childPtr)) {
					hasNeighborOfNeighbor = 1;

					break;
				}
			}

			if (hasNeighborOfNeighbor) {
				childPtr = childPtr->next;

				continue;
			}

			tempDistance = distance(parentPtr->centerX, parentPtr->centerY, childPtr->centerX, childPtr->centerY);

			if (tempDistance < cloestRoomDistance) {
				cloestRoomDistance = tempDistance;

				nearestChildPtr = childPtr;
			}

			childPtr = childPtr->next;
		}

		if (nearestChildPtr) {
			addNeighbor(parentPtr, nearestChildPtr);
		}

		parentPtr = parentPtr->next;
	}
}

void carveTunnels() {
	int i, x, y, x1, y1, wX, wY, placedDoor, isNeighborRoom, positionIndex, nearNeighbor;
	room *neighborPtr, *tempRoom, *parentPtr = getRooms();
	TCOD_path_t pathfinder;
	TCOD_map_t roomMap = TCOD_map_new(WINDOW_WIDTH, WINDOW_HEIGHT);

	while (parentPtr) {
		TCOD_map_clear(roomMap, 1, 1);

		tempRoom = getRooms();

		while (tempRoom) {
			isNeighborRoom = 0;

			TCOD_map_set_properties(roomMap, tempRoom->x, tempRoom->y - 1, 0, 0);
			TCOD_map_set_properties(roomMap, tempRoom->x + tempRoom->width - 1, tempRoom->y, 0, 0);
			TCOD_map_set_properties(roomMap, tempRoom->x + tempRoom->width - 1, tempRoom->y + tempRoom->height - 1, 0, 0);
			TCOD_map_set_properties(roomMap, tempRoom->x, tempRoom->y + tempRoom->height - 1, 0, 0);

			if (parentPtr == tempRoom || isConnectedWith(parentPtr, tempRoom)) {
				tempRoom = tempRoom->next;

				continue;
			}

			for (i = 0; i < parentPtr->numberOfNeighborRooms; i ++) {
				neighborPtr = getRoomViaId(parentPtr->neighborRoomIds[i]);

				if (tempRoom == neighborPtr) {
					isNeighborRoom = 1;

					break;
				}
			}

			if (!isNeighborRoom) {
				for (positionIndex = 0; positionIndex < tempRoom->size; positionIndex ++) {
					x = tempRoom->positionList[positionIndex][0];
					y = tempRoom->positionList[positionIndex][1];

					for (y1 = -1; y1 <= 1; y1 ++) {
						for (x1 = -1; x1 <= 1; x1 ++) {
							TCOD_map_set_properties(roomMap, x + x1, y + y1, 0, 0);
						}
					}
				}
			}

			tempRoom = tempRoom->next;
		}

		pathfinder = TCOD_path_new_using_map(roomMap, 0.f);

		for (i = 0; i < parentPtr->numberOfNeighborRooms; i ++) {
			neighborPtr = getRoomViaId(parentPtr->neighborRoomIds[i]);

			if (isConnectedWith(parentPtr, neighborPtr)) {
				continue;
			}

			if (!TCOD_path_compute(pathfinder, parentPtr->centerX, parentPtr->centerY, neighborPtr->centerX, neighborPtr->centerY)) {
				printf("*FATAL* Can't make a path here.\n");
			}

			placedDoor = 0;
			nearNeighbor = 0;

			while (TCOD_path_walk(pathfinder, &wX, &wY, 0)) {
				TCOD_map_set_properties(LEVEL_MAP, wX, wY, 1, 1);

				for (y1 = -1; y1 < 1; y1 ++) {
					for (x1 = -1; x1 < 1; x1 ++) {
						if ((x1 == -1 && y1 == -1) && (x1 == 1 && y1 == -1) && (x1 == -1 && y1 == 1) & (x1 == 1 && y1 == 1)) {
							continue;
						}

						if (isPositionInRoom(neighborPtr, wX + x1, wY + y1)) {
							nearNeighbor = 1;
						}
					}

					if (nearNeighbor) {
						break;
					}
				}

				if (!isPositionInRoom(parentPtr, wX, wY)) {
					TCOD_map_set_properties(TUNNEL_MAP, wX, wY, 1, 1);

					if (!placedDoor) {
						//if (isPositionInRoom(neighborPtr, wX + x1, wY + y1)) {
						TCOD_map_set_properties(TUNNEL_MAP, wX, wY, 1, 1);
						addRoomDoorPosition(parentPtr, wX, wY);

						placedDoor = 1;
						//}
					}
				}

				if (nearNeighbor) {
					break;
				}
			}

			connectRooms(parentPtr, neighborPtr);
		}

		parentPtr = parentPtr->next;
	}
}

void generateLevel() {
	int spawnPosition[2];
	character *player = getPlayer();

	resetLevel();
	buildDungeon();
	designDungeon();
	connectNeighbors();
	carveTunnels();
	generatePuzzles();
	decorateRooms();
	paintLevel();
	placeItems();
	generateKeys();
	spawnEnemies();
	activateDoors();
	colorRooms();
	placeGrass();
	colorItems();

	if (!STARTING_ROOM) {
		printf("*FATAL* No starting room\n");

		assert(STARTING_ROOM);
	}

	if (player) {
		player->x = START_LOCATION[0];
		player->y = START_LOCATION[1];
		player->itemLight->x = player->x;
		player->itemLight->y = player->y;

		createBoots(player->x, player->y);
		item *ptr = getNewestItem();

		ptr->itemEffectFlags = IS_FLAMING;

		moveActor(player, 1, 0);

		if (LEVEL_NUMBER == 1) {
			plantTorch(player);
		} else {
			getNewSpawnPosition(STARTING_ROOM, spawnPosition);
			createBonfire(spawnPosition[0], spawnPosition[1]);
		}
	}

	resetAllActorsForNewLevel();
	refreshAllLights();
	fadeBackIn();
}
