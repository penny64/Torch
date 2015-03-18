#define MAX_SYSTEMS 100

#include "entities.h"

#ifndef SYSTEM_H
#define SYSTEM_H

typedef struct {
	unsigned int entityMask[MAX_SYSTEMS];
	void (*callback[MAX_SYSTEMS])(World*, unsigned int);
} System;

#endif


void startSystems(void);
void tickSystems(World*);
unsigned int createSystemHandler(World*, unsigned int, void (*)(World*, unsigned int));