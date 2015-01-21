#include "../level.h" //Will fix this later
#include "../lights.h"
#include "../items.h"
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
	
	_c = malloc(sizeof(character));
	_c->x = WINDOW_WIDTH / 2;
	_c->y = WINDOW_HEIGHT / 2;
	_c->vx = 1;
	_c->vy = 0;
	_c->prev = NULL;
	_c->next = NULL;
	_c->fov = copyLevelMap();
	_c->itemLight = createDynamicLight(_c->x, _c->y);
	
	if (CHARACTERS == NULL) {
		CHARACTERS = _c;
	} else {
		_p_c = CHARACTERS;
		
		CHARACTERS->next = _c;
		_c->prev = _p_c;
	}

	return _c;
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

void _actorLogic(character *actor) {
	int nx = actor->x + actor->vx;
	int ny = actor->y + actor->vy;

	if (isPositionWalkable(nx, ny)) {
		actor->x = nx;
		actor->y = ny;
		
		TCOD_map_compute_fov(actor->fov, actor->x, actor->y, 16, 1, FOV_SHADOW);
	}

	_checkForCollisions(actor);
	
	actor->itemLight->x = actor->x;
	actor->itemLight->y = actor->y;

	actor->vx = 0;
	actor->vy = 0;
}

void actorLogic() {
	character *ptr = CHARACTERS;

	while (ptr != NULL) {
		//if (ptr->charType)

		_actorLogic(ptr);

		ptr = ptr->next;
	}
}

void _drawActor(character *actor) {
	drawChar(ACTOR_CONSOLE, actor->x, actor->y, (int)'@', TCOD_color_RGB(255, 255, 255), TCOD_color_RGB(0, 0, 0));
}

void drawActors() {
	character *ptr = CHARACTERS;

	while (ptr != NULL) {
		_drawActor(ptr);

		ptr = ptr->next;
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
