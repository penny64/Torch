#include <stdio.h>

#include "framework/draw.h"
#include "framework/numbers.h"
#include "rects.h"
#include "systems.h"
#include "ui.h"
#include "components.h"

void rectTickHandler(World*, unsigned int);
void rectDrawHandler(World*, unsigned int);


void startRects() {
	World *world = getWorld();

	createSystemHandler(world, COMPONENT_TICK, &rectTickHandler);
	createSystemHandler(world, COMPONENT_DRAW, &rectDrawHandler);
}

void registerRectSystem(World *world, unsigned int entityId, int x, int y, int chr, int direction, float speed, TCOD_color_t foreColor, TCOD_color_t backColor) {
	world->mask[entityId] |= COMPONENT_RECT | COMPONENT_TICK | COMPONENT_DRAW;

	RectComponent *rectComponent = &world->rect[entityId];

	rectComponent->x = x;
	rectComponent->y = y;
	rectComponent->chr = chr;
	rectComponent->exactX = x;
	rectComponent->exactY = y;
	rectComponent->foreColor = foreColor;
	rectComponent->backColor = backColor;

	velocity(rectComponent->velocity, direction, speed);

	printf("Added entity ID=%i to system `rects`.\n", entityId);
}

void rectTickHandler(World *world, unsigned int entityId) {
	RectComponent *rectComponent = &world->rect[entityId];

	rectComponent->exactX += rectComponent->velocity[0];
	rectComponent->exactY += rectComponent->velocity[1];
	rectComponent->x = (int)(rectComponent->exactX + .5);
	rectComponent->y = (int)(rectComponent->exactY + .5);
}

void rectDrawHandler(World *world, unsigned int entityId) {
	RectComponent *rectComponent = &world->rect[entityId];

	drawChar(getUiConsole(), rectComponent->x, rectComponent->y, rectComponent->chr, rectComponent->foreColor, rectComponent->backColor);
}