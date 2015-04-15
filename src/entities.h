#define MAX_ENTITIES 100

#include "components.h"

#ifndef ENTITE_H
#define ENTITE_H

typedef struct World World;

struct World {
	unsigned int mask[MAX_ENTITIES];

	SpellComponent spell[MAX_ENTITIES];
	RectComponent rect[MAX_ENTITIES];
	LightComponent light[MAX_ENTITIES];
	ParticleComponent particle[MAX_ENTITIES];
	AiComponent ai[MAX_ENTITIES];
};

#endif

void startEntities(void);
unsigned int createEntity(World*);
void deleteEntity(World*, unsigned int);
World *getWorld(void);