#include "libtcod.h"
#include "items.h"
#define MAX_ROOMS 6
#define MAX_CONNECTED_ROOMS 5


enum {
	IS_TREASURE_ROOM = 0x01 << 0,
	IS_TORCH_ROOM = 0x01 << 1,
	IS_EXIT_ROOM = 0x01 << 2,
	NEEDS_DOORS = 0x01 << 3,
	IS_RARE_SPAWN = 0x01 << 4,
	IS_LAVA_ROOM = 0x01 << 5,
	IS_START_ROOM = 0x01 << 6,
	IS_PUZZLE_ROOM = 0x01 << 7,
} roomFlag_t;

enum {
	LEVEL_PLAIN = 0,
	LEVEL_KEYTORCH = 1,
} levelType_t;

typedef struct room room;

struct room {
	int id, centerX, centerY, size, numberOfDoorPositions, numberOfConnectedRooms, numberOfOccupiedSpawnPositions;
	int *connectedRooms, *spawnPositions, **positionList, **doorPositions;
	unsigned int flags;
	struct room *next, *prev;
};


TCOD_console_t getLevelConsole(void);
TCOD_console_t getLightConsole(void);
TCOD_console_t getShadowConsole(void);
TCOD_console_t getFogConsole(void);
TCOD_console_t getSeenConsole(void);
TCOD_map_t getLevelMap(void);
TCOD_map_t getTunnelMap(void);
TCOD_map_t copyLevelMap(void);
TCOD_noise_t getFogNoise(void);
void levelSetup(void);
void levelShutdown(void);
void deleteAllRooms(void);
void generateLevel(void);
void completeLevel(void);
void exitLevel(void);
void transitionIsComplete(void);
void setLevel(int);
void blockPosition(int, int);
void unblockPosition(int, int);
void placeItemInRoom(room*, item*);
void createAndPlaceItemInRoom(room*, void(*)(int, int));
int levelLogic(void);
int isPositionWalkable(int, int);
int isLevelComplete(void);
int isTransitionInProgress(void);
int isLevelValid();
int getRandomInt(int, int);
int getLevel(void);
int isRoomConnectedTo(room*, room*);
int isRoomConnectedToId(room*, int);
int *getExitLocation(void);
float getExitWaveDistance(void);
float *getEffectsMap(void);
float getRandomFloat(float, float);
room *createRoom(int, int, unsigned int);
room *getRoomViaId(int);
room *getRoomWithFlags(unsigned int);
