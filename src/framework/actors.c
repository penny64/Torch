#include <stdio.h>

#include "../level.h" //Will fix this later
#include "../lights.h"
#include "../items.h"
#include "../player.h"
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

character *createActor() {
	character *_c, *_p_c;
	
	_c = calloc(1, sizeof(character));
	_c->x = WINDOW_WIDTH / 2;
	_c->y = WINDOW_HEIGHT / 2;
	_c->vx = 0;
	_c->vy = 0;
	_c->prev = NULL;
	_c->next = NULL;
	_c->itemLight = NULL;
	_c->hp = 100;
	_c->fov = copyLevelMap();
	
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

void resetActorForNewLevel(character *actor) {
	actor->fov = copyLevelMap();
	
	//TODO: Delete old torch
	actor->itemLight = createDynamicLight(actor->x, actor->y, actor);
	actor->itemLight->r_tint = 95;
	actor->itemLight->g_tint = 95;
	actor->itemLight->b_tint = 35;
	actor->itemLight->fuelMax = 180;
	actor->itemLight->fuel = actor->itemLight->fuelMax;
}

void _checkForCollisions(character *actor) {
	item *ptr = getItems();

	while (ptr != NULL) {
		if (actor->x == ptr->x && actor->y == ptr->y) {
			itemHandleCharacterCollision(ptr, actor);
		}

		ptr = ptr->next;
	}
}

void _checkIfPositionLit(character *actor) {
	if (!isPositionLit(actor->x, actor->y)) {
		killActor(actor);
	}
}

void _actorLogic(character *actor) {
	if (!actor->hp) {
		return;
	}
	
	int nx = actor->x + actor->vx;
	int ny = actor->y + actor->vy;

	if (nx && ny && isPositionWalkable(nx, ny)) {
		actor->x = nx;
		actor->y = ny;

		if (actor->itemLight) {
			actor->itemLight->fuel --;
		}
		
		TCOD_map_compute_fov(actor->fov, actor->x, actor->y, 16, 1, FOV_SHADOW);
	}

	_checkForCollisions(actor);
	_checkIfPositionLit(actor);
	
	if (actor->itemLight) {
		actor->itemLight->x = actor->x;
		actor->itemLight->y = actor->y;
	}

	actor->vx = 0;
	actor->vy = 0;
}

void actorLogic() {
	character *ptr = CHARACTERS;

	while (ptr != NULL) {
		_actorLogic(ptr);

		ptr = ptr->next;
	}
}

void _drawActor(character *actor) {
	int colorMod = 0;
	
	if (actor->hp <= 0) {
		colorMod = 155;
	}
	
	drawChar(ACTOR_CONSOLE, actor->x, actor->y, (int)'@', TCOD_color_RGB(255 - colorMod, 255 - colorMod, 255 - colorMod), TCOD_color_RGB(0, 0, 0));
}

void drawActors() {
	character *ptr = CHARACTERS;

	while (ptr != NULL) {
		_drawActor(ptr);

		ptr = ptr->next;
	}
}

void killActor(character *actor) {
	actor->hp = 0;

	printf("Killed actor.\n");
	
	if (actor == getPlayer()) {
		showMessage("%cYou die.%c", 15);
	}
}

void actorCleanup() {
	TCOD_console_clear(ACTOR_CONSOLE);
}

character *getActors() {
	return CHARACTERS;
}

TCOD_console_t getActorConsole() {
	return ACTOR_CONSOLE;
}
