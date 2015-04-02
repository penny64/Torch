#include "rects.h"
#include "systems.h"

void rectTickHandler(World*, unsigned int);
void rectDrawHandler(World*, unsigned int);


void startRects() {
	World *world = getWorld();

	createSystemHandler(world, COMPONENT_TICK, &rectTickHandler);
	createSystemHandler(world, COMPONENT_DRAW, &rectDrawHandler);
}

void registerRectSystem(World *world, unsigned int entityId) {
	world->mask[entityId] |= COMPONENT_RECT | COMPONENT_DRAW;

	printf("Added entity ID=%i to system `spells`.\n", entityId);
}

void rectTickHandler(World *world, unsigned int entityId) {
	printf("Here's a rect!\n");
}

void rectDrawHandler(World *world, unsigned int entityId) {
	printf("Here's a rect! Drawing!\n");
}