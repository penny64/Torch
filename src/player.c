#include <stdio.h>

#include "player.h"
#include "framework/actors.h"
#include "framework/input.h"
#include "lights.h"
#include "graphics.h"
//#include "libtcod.h"
//#include "items.h"
#include "level.h"
#include "ui.h"


#define SIZEMOD_TIME_MAX 10;
int SIZEMOD_TIME = SIZEMOD_TIME_MAX;

static struct character *PLAYER_ACTOR = NULL;


void createPlayer() {
	PLAYER_ACTOR = createActor(0, 0);
	
	PLAYER_ACTOR->itemLight->r_tint = 90 - 45;
	PLAYER_ACTOR->itemLight->g_tint = 130;
	PLAYER_ACTOR->itemLight->b_tint = 130;
	PLAYER_ACTOR->itemLight->size = 6;
	PLAYER_ACTOR->itemLight->fuelMax = 350;
	PLAYER_ACTOR->itemLight->fuel = PLAYER_ACTOR->itemLight->fuelMax;
	PLAYER_ACTOR->traitFlags = TORCH_ATTACK_PENALTY;
	
	printf("Created player.\n");
}

character *getPlayer() {
	return PLAYER_ACTOR;
}

int getPlayerMoveCount() {
	return abs(PLAYER_ACTOR->vx) + abs(PLAYER_ACTOR->vy);
}

void _handlePlantTorch() {
	plantTorch(PLAYER_ACTOR);
}

void killPlayer() {
	PLAYER_ACTOR = NULL;
}

void playerLogic() {
	if (!PLAYER_ACTOR) {
		return;
	}
	
	if (isTransitionInProgress()) {
		if (SIZEMOD_TIME > 0) {
			SIZEMOD_TIME --;
		} else {
			SIZEMOD_TIME = SIZEMOD_TIME_MAX;
			
			if (isScreenFadingBackIn()) {
				PLAYER_ACTOR->itemLight->sizeMod += .3f;
			} else {
				PLAYER_ACTOR->itemLight->sizeMod -= .3f;
			}

			if (PLAYER_ACTOR->itemLight->sizeMod < 0) {
				PLAYER_ACTOR->itemLight->sizeMod = 0.f;
			} else if (PLAYER_ACTOR->itemLight->sizeMod > 1) {
				PLAYER_ACTOR->itemLight->sizeMod = 1.f;
			}
		}
	}
}

void playerInputLogic() {
	if (!PLAYER_ACTOR || PLAYER_ACTOR->hp <= 0 || isTransitionInProgress()) {
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
		printf("%i, %i\n", PLAYER_ACTOR->x, PLAYER_ACTOR->y);
	} else if (isCharPressed('d')) {
		if (PLAYER_ACTOR && PLAYER_ACTOR->itemLight) {
			PLAYER_ACTOR->itemLight->fuel = PLAYER_ACTOR->itemLight->fuelMax;
		}
	} else if (isCharPressed('z')) {
		PLAYER_ACTOR->hp = 0;
	}
	
	if (PLAYER_ACTOR->vx || PLAYER_ACTOR->vy) {
		if (!PLAYER_ACTOR->speed) {
			PLAYER_ACTOR->speed = PLAYER_ACTOR->maxSpeed;
			PLAYER_ACTOR->turns = 1;
		}
	}
}
