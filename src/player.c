#include "player.h"
#include "framework/actors.h"
#include "framework/input.h"
#include "lights.h"
#include "libtcod.h"
#include "items.h"
#include "level.h"


static struct character *PLAYER_ACTOR;


void createPlayer() {
	PLAYER_ACTOR = createActor();
	
	PLAYER_ACTOR->itemLight->g_tint = 95;
	PLAYER_ACTOR->itemLight->b_tint = 35;
	PLAYER_ACTOR->itemLight->fuelMax = 180;
	PLAYER_ACTOR->itemLight->fuel = PLAYER_ACTOR->itemLight->fuelMax;
	PLAYER_ACTOR->traitFlags = TORCH_ATTACK_PENALTY;
}

character *getPlayer() {
	return PLAYER_ACTOR;
}

int getPlayerMoveCount() {
	return abs(PLAYER_ACTOR->vx) + abs(PLAYER_ACTOR->vy);
}

void _handlePlantTorch() {
	createPlantedTorch(PLAYER_ACTOR->itemLight->x, PLAYER_ACTOR->itemLight->y, PLAYER_ACTOR->itemLight);

	PLAYER_ACTOR->itemLight = NULL;
}

void playerInputLogic() {
	if (!PLAYER_ACTOR || PLAYER_ACTOR->hp <= 0) {
		return;
	}
	
	if (isTCODCharPressed(TCODK_LEFT)) {
		PLAYER_ACTOR->vx = -1;
	} else if (isTCODCharPressed(TCODK_RIGHT)) {
		PLAYER_ACTOR->vx = 1;
	}

	if (isTCODCharPressed(TCODK_UP)) {
		PLAYER_ACTOR->vy = -1;
	} else if (isTCODCharPressed(TCODK_DOWN)) {
		PLAYER_ACTOR->vy = 1;
	}
	
	if (isCharPressed('1')) {
		PLAYER_ACTOR->vx = -1;
		PLAYER_ACTOR->vy = 1;
	} else if (isCharPressed('2')) {
		PLAYER_ACTOR->vy = 1;
	} else if (isCharPressed('3')) {
		PLAYER_ACTOR->vx = 1;
		PLAYER_ACTOR->vy = 1;
	} else if (isCharPressed('4')) {
		PLAYER_ACTOR->vx = -1;
	} else if (isCharPressed('6')) {
		PLAYER_ACTOR->vx = 1;
	} else if (isCharPressed('7')) {
		PLAYER_ACTOR->vx = -1;
		PLAYER_ACTOR->vy = -1;
	} else if (isCharPressed('8')) {
		PLAYER_ACTOR->vy = -1;
	} else if (isCharPressed('9')) {
		PLAYER_ACTOR->vx = 1;
		PLAYER_ACTOR->vy = -1;
	}

	if (isCharPressed('b')) {
		PLAYER_ACTOR->vx = -1;
		PLAYER_ACTOR->vy = 1;
	} else if (isCharPressed('j')) {
		PLAYER_ACTOR->vy = 1;
	} else if (isCharPressed('n')) {
		PLAYER_ACTOR->vx = 1;
		PLAYER_ACTOR->vy = 1;
	} else if (isCharPressed('h')) {
		PLAYER_ACTOR->vx = -1;
	} else if (isCharPressed('l')) {
		PLAYER_ACTOR->vx = 1;
	} else if (isCharPressed('y')) {
		PLAYER_ACTOR->vx = -1;
		PLAYER_ACTOR->vy = -1;
	} else if (isCharPressed('k')) {
		PLAYER_ACTOR->vy = -1;
	} else if (isCharPressed('u')) {
		PLAYER_ACTOR->vx = 1;
		PLAYER_ACTOR->vy = -1;
	}
	
	if (isCharPressed(' ')) {
		_handlePlantTorch();
	} else if (isCharPressed('c')) {
		completeLevel();
	}
}
