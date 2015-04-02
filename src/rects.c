#include "framework/numbers.h"
#include "rects.h"
#include "systems.h"

void rectTickHandler(World*, unsigned int);
void rectDrawHandler(World*, unsigned int);


void startRects() {
	World *world = getWorld();

	createSystemHandler(world, COMPONENT_TICK, &rectTickHandler);
	createSystemHandler(world, COMPONENT_DRAW, &rectDrawHandler);
}

void registerRectSystem(World *world, unsigned int entityId, int x, int y, int direction, float speed) {
	world->mask[entityId] |= COMPONENT_RECT | COMPONENT_DRAW;

	RectComponent *rectComponent = &world->rect[entityId];

	rectComponent->x = x;
	rectComponent->y = y;
	rectComponent->exactX = x;
	rectComponent->exactY = y;

	velocity(rectComponent->velocity, direction, speed);

	printf("Added entity ID=%i to system `rects`.\n", entityId);
	printf("Velocity: %f, %f\n", rectComponent->velocity[0], rectComponent->velocity[1]);
}

void rectTickHandler(World *world, unsigned int entityId) {
	printf("Here's a rect!\n");
}

void rectDrawHandler(World *world, unsigned int entityId) {
	//printf("Here's a rect! Drawing!\n");
}