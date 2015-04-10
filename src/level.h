#include "libtcod.h"
#include "items.h"
#include "rooms.h"


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
float **getEffectsMap(void);
