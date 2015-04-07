#include "libtcod.h"
#include "items.h"
#include "rooms.h"


enum {
	IS_TREASURE_ROOM = 0x01 << 0,
	IS_TORCH_ROOM = 0x01 << 1,
	IS_EXIT_ROOM = 0x01 << 2,
	NEEDS_DOORS = 0x01 << 3,
	IS_RARE_SPAWN = 0x01 << 4,
	IS_LAVA_ROOM = 0x01 << 5,
	IS_START_ROOM = 0x01 << 6,
	IS_PUZZLE_ROOM = 0x01 << 7,
	IS_FURNACE_ROOM = 0x01 << 8,
} roomFlag_t;

enum {
	LEVEL_PLAIN = 0,
	LEVEL_KEYTORCH = 1,
} levelType_t;


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
void generateLevel(void);
void completeLevel(void);
void exitLevel(void);
void transitionIsComplete(void);
void setLevel(int);
void blockPosition(int, int);
void unblockPosition(int, int);
int levelLogic(void);
int isPositionWalkable(int, int);
int isLevelComplete(void);
int isTransitionInProgress(void);
int isLevelValid();
int getLevel(void);
int *getExitLocation(void);
int *getRoomMap(void);
float getExitWaveDistance(void);
float *getEffectsMap(void);
