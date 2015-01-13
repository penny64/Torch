#include <stdlib.h>

#include "display.h"
#include "logging.h"
#include "actors.h"
#include "draw.h"


character *createActor() {
	character *_c, *_p_c;
	
	_c = malloc(sizeof(character));
	_c->x = 0;
	_c->y = 0;
	_c->vx = 0;
	_c->vy = 0;
	_c->prev = NULL;
	_c->next = NULL;
	
	if (CHARACTERS == NULL) {
		CHARACTERS = _c;
	} else {
		_p_c = CHARACTERS;
		
		CHARACTERS->next = _c;
		_c->prev = _p_c;
	}

	return _c;
}

void _actorLogic(character *actor) {
	actor->x += actor->vx;
	actor->y += actor->vy;

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