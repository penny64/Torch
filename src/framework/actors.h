#include <stdlib.h>

#include "libtcod.h"
#include "../items.h"

#ifndef ACTORS_H
#define ACTORS_H

enum {
	RANDOM_WALK = 0x01 << 0,
	WORM_WALK = 0x01 << 1,
	DROP_LIGHT_ON_DEATH = 0x01 << 2,
	EXPLODE_ON_DEATH = 0x01 << 3,
	DROP_VOID_ON_DEATH = 0x01 << 4,
	IS_VOID_WORM = 0x01 << 5,
	IS_IMMUNE_TO_DARKNESS = 0x01 << 6,
} aiFlag_t;

enum {
	TORCH_ATTACK_PENALTY = 0x01 << 0,
	FREE = 0x01 << 1,
	FREE2 = 0x01 << 2,
	FREE3 = 0x01 << 3,
} traitFlags_t;

enum {
	IS_STANDING = 0x01 << 0,
	IS_SWINGING = 0x01 << 1,
	IS_STABBING = 0x01 << 2,
	IS_KICKING = 0x01 << 3,
	IS_CRAWLING = 0x01 << 4,
	IS_HOLDING_LODGED_WEAPON = 0x01 << 5,
	IS_STUNNED = 0x01 << 6,
} stanceFlags_t;

typedef struct character character;

struct character {
	int x, y, vx, vy, hp, stanceDelay, speed, maxSpeed, turns, chr, sightRange, numberOfItems;
	struct character *next, *prev;
	struct light *itemLight;
	struct item *inventory[4];
	unsigned int aiFlags, traitFlags, stanceFlags;
	TCOD_color_t foreColor, backColor;
	TCOD_console_t fov;
};

#endif

character *createActor(int, int);
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
void pickUpItem(character*, struct item*);
struct item *actorGetItemWithFlag(character*, unsigned int);
