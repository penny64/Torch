#include <stdlib.h>

#include "libtcod.h"


typedef struct character character;

struct character {
	int x, y, vx, vy, hp;
	struct character *next, *prev;
	TCOD_console_t fov;
};

character *createActor(void);
character *getActors(void);
TCOD_console_t getActorConsole(void);
void actorLogic(void);
void drawActors(void);
void actorSetup(void);
void actorCleanup(void);
