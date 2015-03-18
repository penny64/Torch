#include <stdio.h>

#include "entities.h"

World *WORLD = NULL;


void startEntities() {
	WORLD = calloc(sizeof(World*), MAX_ENTITIES);
}

World *getWorld() {
	return WORLD;
}

unsigned int createEntity(World *world) {
	unsigned int entityId;

	for (entityId = 0; entityId < MAX_ENTITIES; entityId ++) {
		if (world->mask[entityId] == COMPONENT_NONE) {
			printf("Created new entity ID=%i\n", entityId);

			return entityId;
		}
	}

	printf("*FATAL* Out of free entities\n");

	return MAX_ENTITIES;
}

void destroyEntity(World *world, unsigned int entityId) {
	world->mask[entityId] = COMPONENT_NONE;
}