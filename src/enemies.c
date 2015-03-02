#include "enemies.h"
#include "framework/actors.h"
#include "framework/input.h"
#include "lights.h"
#include "libtcod.h"
#include "items.h"
#include "level.h"


void createBat(int x, int y) {
	character *actor = createActor(x, y);
	
	actor->hp = 25;
	actor->sightRange = 4;
	actor->aiFlags = RANDOM_WALK | DROP_LIGHT_ON_DEATH;
	actor->chr = (int)'^';
	actor->foreColor = TCOD_color_RGB(205, 25, 25);
	actor->itemLight->x = x;
	actor->itemLight->y = y;
	actor->itemLight->r_tint = 125;
	actor->itemLight->size = 4;
	actor->itemLight->fuelMax = 99999;
	actor->itemLight->fuel = actor->itemLight->fuelMax;
}

void createVoidWorm(int x, int y) {
	character *actor = createActor(x, y);

	actor->hp = 15;
	actor->sightRange = 12;
	actor->maxSpeed = 8;
	actor->aiFlags = IS_VOID_WORM | RANDOM_WALK | DROP_VOID_ON_DEATH | IS_IMMUNE_TO_DARKNESS;
	actor->chr = (int)'O';
	actor->foreColor = TCOD_color_RGB(25, 205, 25);
	actor->itemLight->x = x;
	actor->itemLight->y = y;
	actor->itemLight->r_tint = 125;
	actor->itemLight->b_tint = 125;
	actor->itemLight->size = 6;
	actor->itemLight->fuelMax = 99999;
	actor->itemLight->fuel = actor->itemLight->fuelMax;
	actor->stanceFlags = IS_STANDING;
}

void createVoidWormTail(int x, int y) {
	character *actor = createActor(x, y);

	actor->hp = 5;
	actor->sightRange = 0;
	actor->aiFlags = 0x0;
	actor->chr = (int)'o';
	actor->foreColor = TCOD_color_RGB(15, 100, 15);
	//deleteDynamicLight(actor->itemLight);
	//actor->itemLight = NULL;
	actor->stanceFlags = IS_STANDING;
	actor->itemLight->r_tint = 0;
	actor->itemLight->g_tint = 35;
	actor->itemLight->b_tint = 0;
	actor->itemLight->size = 3;
	actor->itemLight->fuelMax = 12;
	actor->itemLight->fuel = actor->itemLight->fuelMax;
}
