#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "systems.h"
#include "entities.h"


System *SYSTEMS = NULL;


void startSystems() {
	SYSTEMS = calloc(sizeof(System), 1);
}

unsigned int createSystemHandler(World *world, unsigned int entityMask, void (*callback)(World*, unsigned int)) {
	unsigned int systemId;

	for (systemId = 0; systemId < MAX_SYSTEMS; systemId ++) {
		if (SYSTEMS->entityMask[systemId] == COMPONENT_NONE) {
			SYSTEMS->entityMask[systemId] = entityMask;
			SYSTEMS->callback[systemId] = callback;

			printf("Created new system ID=%i\n", systemId);

			return systemId;
		}
	}

	printf("*FATAL* Out of free systems\n");

	return MAX_SYSTEMS;
}

void tickSystems(World *world) {
	unsigned int systemId, entityId;

	for (systemId = 0; systemId < MAX_SYSTEMS; systemId ++) {
		if (SYSTEMS->entityMask[systemId] != COMPONENT_NONE) {
			for (entityId = 0; entityId < MAX_ENTITIES; entityId ++) {
				if (world->mask[entityId] & SYSTEMS->entityMask[systemId]) {
					SYSTEMS->callback[systemId](world, entityId);
				}
			}
		}
	}
}

void tickSystemsWithMask(World *world, unsigned int mask) {
	unsigned int systemId, entityId;

	for (systemId = 0; systemId < MAX_SYSTEMS; systemId ++) {
		if (SYSTEMS->entityMask[systemId] != COMPONENT_NONE && SYSTEMS->entityMask[systemId] & mask) {
			for (entityId = 0; entityId < MAX_ENTITIES; entityId ++) {
				if (world->mask[entityId] & SYSTEMS->entityMask[systemId]) {
					SYSTEMS->callback[systemId](world, entityId);
				}
			}
		}
	}
}

void tickSystemsWithMaskForEntity(World *world, unsigned int entityId, unsigned int mask) {
	unsigned int systemId;

	for (systemId = 0; systemId < MAX_SYSTEMS; systemId ++) {
		if (SYSTEMS->entityMask[systemId] != COMPONENT_NONE && SYSTEMS->entityMask[systemId] & mask) {
			if (world->mask[entityId] & SYSTEMS->entityMask[systemId]) {
				SYSTEMS->callback[systemId](world, entityId);
			}
		}
	}
}