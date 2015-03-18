#define MAX_SYSTEMS 100

#include "entities.h"

#ifndef SYSTEM_H
#define SYSTEM_H

typedef struct {
	unsigned int entityMask;
	void (*callback)(World*);
} System;

#endif


void startSystems(void);
void createSystem(unsigned int, void (*)(World*));