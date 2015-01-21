#include <stdlib.h>

#include "libtcod.h"

#ifndef SOME_HEADER_GUARD_WITH_UNIQUE_NAME
#define SOME_HEADER_GUARD_WITH_UNIQUE_NAME

typedef struct character character;

struct character {
	int x, y, vx, vy, hp;
	struct character *next, *prev;
	struct light *itemLight;
	TCOD_console_t fov;
};

#endif

character *createActor(void);
character *getActors(void);
TCOD_console_t getActorConsole(void);
void actorLogic(void);
void drawActors(void);
void actorSetup(void);
void actorCleanup(void);
