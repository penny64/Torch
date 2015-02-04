#include "enemies.h"
#include "framework/actors.h"
#include "framework/input.h"
#include "lights.h"
#include "libtcod.h"
#include "items.h"
#include "level.h"


void createBat(int x, int y) {
	character *actor = createActor();
	
	actor->x = x;
	actor->y = y;
	actor->hp = 25;
	actor->aiFlags = RANDOM_WALK | DROP_LIGHT_ON_DEATH;
	actor->chr = (int)'^';
	actor->foreColor = TCOD_color_RGB(205, 25, 25);
	actor->itemLight->r_tint = 125;
	actor->itemLight->size = 4;
	actor->itemLight->fuelMax = 99999;
	actor->itemLight->fuel = actor->itemLight->fuelMax;
}

