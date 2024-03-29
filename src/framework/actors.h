#include <stdlib.h>

#include "libtcod.h"
#include "../items.h"

#define MAX_INVENTORY_ITEMS 10

#ifndef ACTORS_H
#define ACTORS_H

enum {
	FREE_OR_SOMETHING = 0x01 << 0,
	WORM_WALK = 0x01 << 1,
	DROP_LIGHT_ON_DEATH = 0x01 << 2,
	EXPLODE_ON_DEATH = 0x01 << 3,
	DROP_VOID_ON_DEATH = 0x01 << 4,
	IS_VOID_WORM = 0x01 << 5,
	IS_VOID_WORM_TAIL = 0x01 << 6,
	IS_IMMUNE_TO_DARKNESS = 0x01 << 7,
	IS_ALIVE = 0x01 << 8,
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
	IS_PUNCHING = 0x01 << 7,
	IS_STUCK_WITH_LODGED_WEAPON = 0x01 << 8,
	IS_MOVING = 0x01 << 9,
	IS_CASTING = 0x01 << 10,
	IS_RECOVERING = 0x01 << 11,
} stanceFlags_t;

typedef struct character character;

struct character {
	int x, y, lastX, lastY, vx, vy, hp, hpMax, statLevel, statLuck, statStrength, statDefense, statStabCount, chr, sightRange, numberOfItems;
	float turns, statSpeed, statStability, delay;
	struct character *next, *prev;
	struct light *itemLight;
	struct item *inventory[MAX_INVENTORY_ITEMS];
	unsigned int entityId, aiFlags, traitFlags, stanceFlags, nextStanceFlagsToAdd, nextStanceFlagsToRemove;
	TCOD_color_t foreColor, backColor;
	TCOD_path_t path;
	TCOD_map_t fov;
};

#endif

character *createActor(int, int);
character *getActors(void);
character *getActorViaId(unsigned int);
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
void dropItem(character*, struct item*);
void setStance(character*, unsigned int);
void unsetStance(character*, unsigned int);
void setDelay(character*, float);
void setFutureStanceToAdd(character*, unsigned int);
void setFutureStanceToRemove(character*, unsigned int);
void moveActor(character*, int, int);
int walkActor(character*, int, int);
void walkActorPath(character*);
struct item *actorGetItemWithFlag(character*, unsigned int);
float getMovementCost(character*);
int getActorLevel(character*);
int getActorLuck(character*);
int getActorStrength(character*);
float getActorSpeed(character*);
int getActorDefense(character*);
float getActorStability(character*);
