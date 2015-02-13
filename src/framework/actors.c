#include <stdio.h>

#include "../level.h" //Will fix this later
#include "../lights.h"
#include "../items.h"
#include "../player.h"
#include "../combat.h"
#include "../ui.h"
#include "display.h"
#include "logging.h"
#include "actors.h"
#include "draw.h"


TCOD_console_t ACTOR_CONSOLE;
static character *CHARACTERS = NULL;


void actorSetup() {
	ACTOR_CONSOLE = TCOD_console_new(WINDOW_WIDTH, WINDOW_HEIGHT);
}

void actorsShutdown() {
	character *next, *ptr = CHARACTERS;
	
	printf("Cleaning up actors...\n");
	
	while (ptr != NULL) {
		next = ptr->next;
		
		free(ptr);

		ptr = next;
	}
}

character *createActor() {
	character *_c, *_p_c;
	
	_c = calloc(1, sizeof(character));
	_c->x = WINDOW_WIDTH / 2;
	_c->y = WINDOW_HEIGHT / 2;
	_c->vx = 0;
	_c->vy = 0;
	_c->sightRange = 16;
	_c->prev = NULL;
	_c->next = NULL;
	_c->itemLight = createDynamicLight(_c->x, _c->y, _c);
	_c->hp = 100;
	_c->fov = copyLevelMap();
	_c->chr = (int)'@';
	_c->foreColor = TCOD_color_RGB(255, 255, 255);
	_c->backColor = TCOD_color_RGB(255, 0, 0);
	
	if (CHARACTERS == NULL) {
		CHARACTERS = _c;
	} else {
		_p_c = CHARACTERS;
		
		while (_p_c->next) {
			_p_c = _p_c->next;
		}
		
		_p_c->next = _c;
		_c->prev = _p_c;
	}

	return _c;
}

void deleteActor(character *chr) {
	if (chr == getPlayer()) {
		printf("We're deleting the player for some fucking reason\n");
	}

	if (chr->itemLight) {
		deleteDynamicLight(chr->itemLight);
	}

	if (chr == CHARACTERS) {
		CHARACTERS = chr->next;
	} else {
		chr->prev->next = chr->next;

		if (chr->next) {
			chr->next->prev = chr->prev;
		}
	}

	free(chr);
}

void deleteEnemies() {
	character *next = NULL, *ptr = CHARACTERS;

	if (CHARACTERS == NULL) {
		printf("No enemies to delete.\n");

		return;
	}

	while (ptr != NULL) {
		next = ptr->next;

		if (ptr != getPlayer()) {
			deleteActor(ptr);
		}

		ptr = next;
	}
}

void _resetActorForNewLevel(character *actor) {
	if (actor->fov) {
		TCOD_map_delete(actor->fov);
	}
	
	actor->fov = copyLevelMap();

	if (actor->itemLight) {
		resetLight(actor->itemLight);
	}

	TCOD_map_compute_fov(actor->fov, actor->x, actor->y, actor->sightRange, 1, FOV_SHADOW);
}

void resetAllActorsForNewLevel() {
	character *ptr = CHARACTERS;
	
	while (ptr != NULL) {
		_resetActorForNewLevel(ptr);
		
		ptr = ptr->next;
	}
}

void plantTorch(character *actor) {
	if (!actor->itemLight) {
		printf("Trying to plant a torch that isn't held.\n");

		return;
	}

	createPlantedTorch(actor->x, actor->y, actor->itemLight);

	actor->itemLight = NULL;
}

void _checkForItemCollisions(character *actor) {
	item *ptr = getItems();

	while (ptr != NULL) {
		if (actor->x == ptr->x && actor->y == ptr->y) {
			itemHandleCharacterCollision(ptr, actor);
		}

		ptr = ptr->next;
	}
}

int _checkIfPositionLit(character *actor) {
	if (!isPositionLit(actor->x, actor->y)) {
		printf("Actor is in unlit position\n");
		killActor(actor);

		return 1;
	}

	return 0;
}

void _actorAi(character *actor) {
	if (actor->aiFlags & RANDOM_WALK) {
		actor->vx += getRandomInt(-1, 1);
		actor->vy += getRandomInt(-1, 1);
	} else if (actor->aiFlags & WORM_WALK) {
		//TODO: Potentially store AI info in a different struct,
		//a linked list containing AI states per AI type

		//TODO: Move to its own function

		character *player = getPlayer();

		if (!player) {
			return;
		}

		if (TCOD_map_is_in_fov(actor->fov, player->x, player->y)) {
			printf("Player is visible!\n");
		}

	}
}

void _actorLogic(character *actor) {
	if (!actor->hp) {
		return;
	}
	
	character *ptr = CHARACTERS;
	int hitActor = 0;
	int nx = actor->x + actor->vx;
	int ny = actor->y + actor->vy;
	
	while (ptr != NULL) {
		if (ptr == actor || ptr->hp <= 0) {
			ptr = ptr->next;
			
			continue;
		}
		
		if (ptr->x == nx && ptr->y == ny) {
			hitActor = 1;
			
			break;
		}

		ptr = ptr->next;
	}

	if (hitActor) {
		if (meleeAttack(actor, ptr)) {
			return;
		}
	} else if ((actor->vx || actor->vy) && isPositionWalkable(nx, ny)) {
		actor->x = nx;
		actor->y = ny;

		if (actor->itemLight) {
			actor->itemLight->fuel -= getLevel();
		}

		TCOD_map_compute_fov(actor->fov, actor->x, actor->y, actor->sightRange, 1, FOV_SHADOW);
	}

	_checkForItemCollisions(actor);
	if (_checkIfPositionLit(actor)) {
		return;
	}
	
	if (actor->itemLight) {
		actor->itemLight->x = actor->x;
		actor->itemLight->y = actor->y;
	}

	actor->vx = 0;
	actor->vy = 0;
}

void actorLogic() {
	character *next = NULL, *ptr = CHARACTERS;

	while (ptr != NULL) {
		next = ptr->next;

		_actorAi(ptr);
		_actorLogic(ptr);

		ptr = next;
	}
}

void _drawActor(character *actor) {
	//int colorMod = 0;
	
	if (actor->hp <= 0 || (isTransitionInProgress() && actor->itemLight && actor->itemLight->sizeMod == 0)) {
		return;
	}
	
	drawChar(ACTOR_CONSOLE, actor->x, actor->y, actor->chr, actor->foreColor, actor->backColor);
}

void drawActors() {
	character *ptr = CHARACTERS;

	while (ptr != NULL) {
		_drawActor(ptr);

		ptr = ptr->next;
	}
}

void killActor(character *actor) {
	TCOD_console_t levelConsole = getLevelConsole();
	
	actor->hp = 0;
	if (actor->itemLight) {
		if (actor->aiFlags & DROP_LIGHT_ON_DEATH) {
			actor->itemLight->owner = NULL;

			if (actor->itemLight->fuel > (int)(actor->itemLight->fuelMax * .25f)) {
				actor->itemLight->fuel = (int)(actor->itemLight->fuelMax * .25f);
			}
		} else {
			deleteDynamicLight(actor->itemLight);
		}
		
		actor->itemLight = NULL;
	}
	
	drawChar(levelConsole, actor->x, actor->y, actor->chr, TCOD_color_RGB(actor->foreColor.r * .55f, actor->foreColor.g * .55f, actor->foreColor.b * .55f), actor->backColor);

	printf("Killed actor.\n");
	
	if (actor == getPlayer()) {
		showMessage("%cYou die.%c", 15);
	}
}

void actorCleanup() {
	character *nextChar, *ptr = CHARACTERS;
	int freedActors = 0;

	while (ptr != NULL) {
		nextChar = ptr->next;
		
		if (ptr->hp <= 0) {
			deleteActor(ptr);

			freedActors ++;
		}

		ptr = nextChar;
	}

	if (freedActors) {
		printf("%i actor(s) freed.\n", freedActors);
	}

	TCOD_console_clear(ACTOR_CONSOLE);
}

character *getActors() {
	return CHARACTERS;
}

TCOD_console_t getActorConsole() {
	return ACTOR_CONSOLE;
}
