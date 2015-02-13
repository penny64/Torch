#include <stdlib.h>

#include "libtcod.h"

#ifndef ACTORS_H
#define ACTORS_H


enum {
	RANDOM_WALK = 0x01 << 0,
	WORM_WALK = 0x01 << 1,
	DROP_LIGHT_ON_DEATH = 0x01 << 2,
	EXPLODE_ON_DEATH = 0x01 << 3,
	DROP_VOID_ON_DEATH = 0x01 << 4,
} aiFlag_t;

enum {
	TORCH_ATTACK_PENALTY = 0x01,
	FREE = 0x02,
	FREE2 = 0x04,
	FREE3 = 0x08,
} traitFlags_t;

typedef struct character character;

struct character {
	int x, y, vx, vy, hp, chr, sightRange;
	struct character *next, *prev;
	struct light *itemLight;
	unsigned int aiFlags, traitFlags;
	TCOD_color_t foreColor, backColor;
	TCOD_console_t fov;
};

#endif

character *createActor(void);
character *getActors(void);
TCOD_console_t getActorConsole(void);
void resetAllActorsForNewLevel();
void actorLogic(void);
void drawActors(void);
void actorSetup(void);
void actorsShutdown(void);
void actorCleanup(void);
void deleteEnemies(void);
void plantTorch(character*);
void killActor(character*);
