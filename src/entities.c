#include <stdio.h>

#include "entities.h"
#include "systems.h"

World *WORLD = NULL;


void startEntities() {
	WORLD = calloc(sizeof(World), 1);
}

World *getWorld() {
	return WORLD;
}

unsigned int createEntity(World *world) {
	unsigned int entityId;

	for (entityId = 0; entityId < MAX_ENTITIES; entityId ++) {
		if (world->mask[entityId] == COMPONENT_NONE) {
			world->mask[entityId] = COMPONENT_OCCUPIED;

			printf("Created new entity ID=%i\n", entityId);

			return entityId;
		}
	}

	printf("*FATAL* Out of free entities\n");

	return MAX_ENTITIES;
}

void deleteEntity(World *world, unsigned int entityId) {
	tickSystemsWithMaskForEntity(world, entityId, EVENT_DELETED);

	printf("Deleted entity ID=%i\n", entityId);

	world->mask[entityId] = COMPONENT_NONE;
}