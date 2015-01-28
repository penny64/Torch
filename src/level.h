#include "libtcod.h"
#define MAX_ROOMS 6


TCOD_console_t getLevelConsole(void);
TCOD_console_t getLightConsole(void);
TCOD_console_t getShadowConsole(void);
TCOD_console_t getFogConsole(void);
TCOD_console_t getSeenConsole(void);
TCOD_map_t getLevelMap(void);
TCOD_map_t copyLevelMap(void);
TCOD_map_t copyLevelMap(void);
TCOD_noise_t getFogNoise(void);
void levelSetup(void);
void generateLevel(void);
int isPositionWalkable(int, int);
int getRandomInt(int, int);
float *getEffectsMap(void);
float getRandomFloat(float, float);
