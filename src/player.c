#include <stdio.h>

#include "player.h"
#include "framework/actors.h"
#include "framework/input.h"
#include "lights.h"
#include "graphics.h"
//#include "libtcod.h"
//#include "items.h"
#include "level.h"
#include "entities.h"
#include "spells.h"
#include "ui.h"


#define SIZEMOD_TIME_MAX 10;
int SIZEMOD_TIME = SIZEMOD_TIME_MAX;

static struct character *PLAYER_ACTOR = NULL;


void createPlayer() {
	PLAYER_ACTOR = createActor(1, 1);
	
	PLAYER_ACTOR->itemLight->r_tint = 20;
	PLAYER_ACTOR->itemLight->g_tint = 20;
	PLAYER_ACTOR->itemLight->b_tint = 20;
	PLAYER_ACTOR->itemLight->brightness = .35;
	PLAYER_ACTOR->itemLight->size = 6;
	PLAYER_ACTOR->itemLight->fuelMax = 650;
	PLAYER_ACTOR->itemLight->fuel = PLAYER_ACTOR->itemLight->fuelMax;
	PLAYER_ACTOR->traitFlags = TORCH_ATTACK_PENALTY;

	registerSpellSystem(getWorld(), PLAYER_ACTOR->entityId);
	addSpell(getWorld(), PLAYER_ACTOR->entityId, SPELL_FIREBALL);
	
	printf("Created player.\n");
}

character *getPlayer() {
	return PLAYER_ACTOR;
}

int getPlayerMoveCount() {
	return PLAYER_ACTOR->turns;
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

	if (isMenuOpen()) {
		uiInput();

		return;
	}
	
	if (isTCODCharPressed(TCODK_LEFT)) {
		moveActor(PLAYER_ACTOR, -1, 0);
	} else if (isTCODCharPressed(TCODK_RIGHT)) {
		moveActor(PLAYER_ACTOR, 1, 0);
	}

	if (isTCODCharPressed(TCODK_UP)) {
		moveActor(PLAYER_ACTOR, 0, -1);
	} else if (isTCODCharPressed(TCODK_DOWN)) {
		moveActor(PLAYER_ACTOR, 0, 1);
	}
	
	if (isCharPressed('1')) {
		moveActor(PLAYER_ACTOR, -1, 1);
		//PLAYER_ACTOR->vx = -1;
		//PLAYER_ACTOR->vy = 1;
	} else if (isCharPressed('2')) {
		moveActor(PLAYER_ACTOR, 0, 1);
		//PLAYER_ACTOR->vy = 1;
	} else if (isCharPressed('3')) {
		moveActor(PLAYER_ACTOR, 1, 1);
		//PLAYER_ACTOR->vx = 1;
		//PLAYER_ACTOR->vy = 1;
	} else if (isCharPressed('4')) {
		moveActor(PLAYER_ACTOR, -1, 0);
		//PLAYER_ACTOR->vx = -1;
	} else if (isCharPressed('6')) {
		//PLAYER_ACTOR->vx = 1;
		moveActor(PLAYER_ACTOR, 1, 0);
	} else if (isCharPressed('7')) {
		moveActor(PLAYER_ACTOR, -1, -1);
	} else if (isCharPressed('8')) {
		moveActor(PLAYER_ACTOR, 0, -1);
	} else if (isCharPressed('9')) {
		moveActor(PLAYER_ACTOR, 1, -1);
	}

	if (isCharPressed('b')) {
		moveActor(PLAYER_ACTOR, -1, 1);
	} else if (isCharPressed('j')) {
		moveActor(PLAYER_ACTOR, 0, 1);
	} else if (isCharPressed('n')) {
		moveActor(PLAYER_ACTOR, 1, 1);
	} else if (isCharPressed('h')) {
		moveActor(PLAYER_ACTOR, -1, 0);
	} else if (isCharPressed('l')) {
		moveActor(PLAYER_ACTOR, 1, 0);
	} else if (isCharPressed('y')) {
		moveActor(PLAYER_ACTOR, -1, -1);
	} else if (isCharPressed('k')) {
		moveActor(PLAYER_ACTOR, 0, -1);
	} else if (isCharPressed('u')) {
		moveActor(PLAYER_ACTOR, 1, -1);
	}
	
	if (isCharPressed('.') || isCharPressed('5')) {
		setDelay(PLAYER_ACTOR, 1);
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
	
	/*if (PLAYER_ACTOR->vx || PLAYER_ACTOR->vy) {
		if (!PLAYER_ACTOR->delay) {
			PLAYER_ACTOR->delay = PLAYER_ACTOR->statSpeed;
			PLAYER_ACTOR->turns = 1;
		}
	}*/
}
