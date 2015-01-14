#include <stdlib.h>

#include "libtcod.h"


typedef struct character character;

struct character {
	int x, y, vx, vy, hp;
	struct character *next, *prev;
};

character *createActor(void);
TCOD_console_t getActorConsole(void);
void actorLogic(void);
void drawActors(void);
void actorSetup(void);
void actorCleanup(void);

static character *CHARACTERS = NULL;
