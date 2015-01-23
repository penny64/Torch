#include <stdlib.h>

#include "libtcod.h"

#ifndef ACTORS_H
#define ACTORS_H

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
void killActor(character*);