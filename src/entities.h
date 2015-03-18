#define MAX_ENTITIES 100

#include "components.h"

#ifndef ENTITE_H
#define ENTITE_H

typedef struct {
	unsigned int mask[MAX_ENTITIES];

	SpellComponent spell[MAX_ENTITIES];
} World;

#endif

void startEntities(void);
unsigned int createEntity(World*);
World *getWorld(void);