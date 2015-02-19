#include "libtcod.h"
#define MAX_ROOMS 4
#define MAX_CONNECTED_ROOMS 5


typedef struct room room;

struct room {
	int id, numberOfConnectedRooms, size, **positionList;
	int connectedRooms[MAX_CONNECTED_ROOMS];
	struct room *next, *prev;
	struct character *owner;
};


TCOD_console_t getLevelConsole(void);
TCOD_console_t getLightConsole(void);
TCOD_console_t getShadowConsole(void);
TCOD_console_t getFogConsole(void);
TCOD_console_t getSeenConsole(void);
TCOD_map_t getLevelMap(void);
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
int getRandomInt(int, int);
int getLevel(void);
int isConnectedTo(room*, room*);
int *getExitLocation(void);
float getExitWaveDistance(void);
float *getEffectsMap(void);
float getRandomFloat(float, float);
room *createRoom(int, int);
