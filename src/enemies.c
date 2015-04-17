#include "enemies.h"
#include "framework/actors.h"
#include "framework/input.h"
#include "lights.h"
#include "ai.h"
#include "libtcod.h"
#include "items.h"
#include "level.h"


void createBat(int x, int y) {
	character *actor = createActor(x, y);

	registerAi(getWorld(), actor->entityId);
	registerAiWander(getWorld(), actor->entityId);
	
	actor->hp = 15;
	actor->hpMax = 15;
	actor->sightRange = 4;
	actor->statSpeed = 2.5;
	actor->aiFlags = DROP_LIGHT_ON_DEATH | IS_ALIVE;
	actor->chr = (int)'^';
	actor->foreColor = TCOD_color_RGB(205, 25, 25);
	actor->itemLight->r_tint = 125;
	actor->itemLight->size = 4;
	actor->itemLight->fuelMax = 99999;
	actor->itemLight->fuel = actor->itemLight->fuelMax;
}

void createRagdoll(int x, int y) {
	character *actor = createActor(x, y);

	registerAi(getWorld(), actor->entityId);
	registerAiPatrol(getWorld(), actor->entityId);
	registerAiTrack(getWorld(), actor->entityId);
	
	actor->hp = 25;
	actor->hpMax = 25;
	actor->sightRange = 4;
	actor->statSpeed = 4.3;
	actor->aiFlags = IS_ALIVE | IS_IMMUNE_TO_DARKNESS;
	actor->chr = (int)'&';
	actor->foreColor = TCOD_color_RGB(205, 205, 205);

	deleteDynamicLight(actor->itemLight);
	actor->itemLight = NULL;

	/*actor->itemLight->r_tint = 50;
	actor->itemLight->g_tint = 50;
	actor->itemLight->b_tint = 50;
	actor->itemLight->size = 3;
	actor->itemLight->fuelMax = 99999;
	actor->itemLight->fuel = actor->itemLight->fuelMax;*/
}

void createVoidWorm(int x, int y) {
	character *actor = createActor(x, y);

	registerAi(getWorld(), actor->entityId);
	registerAiWander(getWorld(), actor->entityId);

	actor->hp = 15;
	actor->hpMax = 15;
	actor->sightRange = 12;
	actor->statSpeed = 18;
	actor->aiFlags = IS_VOID_WORM | DROP_VOID_ON_DEATH | IS_IMMUNE_TO_DARKNESS | IS_ALIVE;
	actor->chr = 128;
	actor->foreColor = TCOD_color_RGB(25, 205, 25);

	deleteDynamicLight(actor->itemLight);
	actor->itemLight = NULL;
	/*actor->itemLight->r_tint = 125;
	actor->itemLight->b_tint = 125;
	actor->itemLight->brightness = .35;
	actor->itemLight->size = 3;
	actor->itemLight->fuelMax = 99999;
	actor->itemLight->fuel = actor->itemLight->fuelMax;
	actor->itemLight->flickerRate = 0;*/
	actor->stanceFlags = IS_STANDING;
}

void createVoidWormTail(int x, int y) {
	character *actor = createActor(x, y);

	actor->hp = 5;
	actor->hpMax = 5;
	actor->sightRange = 0;
	actor->aiFlags = 0x0;
	actor->chr = 177;
	actor->foreColor = TCOD_color_RGB(15, 100, 15);
	//deleteDynamicLight(actor->itemLight);
	//actor->itemLight = NULL;
	actor->stanceFlags = IS_CRAWLING;
	actor->itemLight->r_tint = 0;
	actor->itemLight->g_tint = 35;
	actor->itemLight->b_tint = 0;
	actor->itemLight->flickerRate = .2;
	actor->itemLight->size = 3;
	actor->itemLight->fuelMax = 12;
	actor->itemLight->fuel = actor->itemLight->fuelMax;
}
