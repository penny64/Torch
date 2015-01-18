#include "framework/display.h"
#include "level.h"
#include "lights.h"


static light *LIGHTS = NULL;


light *createLight(int x, int y) {
	light *_c, *_p_c;
	
	_c = malloc(sizeof(light));
	_c->x = WINDOW_WIDTH / 2;
	_c->y = WINDOW_HEIGHT / 2;
	//_c->vx = 0;
	//_c->vy = 0;
	_c->prev = NULL;
	_c->next = NULL;
	_c->fov = copyLevelMap();
	
	if (LIGHTS == NULL) {
		LIGHTS = _c;
	} else {
		_p_c = LIGHTS;
		
		LIGHTS->next = _c;
		_c->prev = _p_c;
	}

	return _c;
}

void _drawLight(light *light) {
	
}

void drawLights() {
	ligth *ptr = LIGHTS;

	while (ptr != NULL) {
		_drawLight(ptr);

		ptr = ptr->next;
	}
}