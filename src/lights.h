#include "framework/actors.h"

#include <stdint.h>

#ifndef LIGHTS_H
#define LIGHTS_H

typedef struct light light;

struct light {
	int x, y, fuel, fuelMax, size, noTint;
	uint8_t r_tint, g_tint, b_tint;
	unsigned int entityId;
	float sizeMod, brightness, flickerRate;
	struct light *next, *prev;
	struct character *owner;
	TCOD_map_t fov;
	TCOD_map_t lightMap;
};

#endif

TCOD_map_t getLightMap(void);
TCOD_console_t getDynamicLightConsole(void);
light *createLight(int, int);
light *createDynamicLight(int, int, character*);
light *getLights(void);
light *getDynamicLights(void);
light *getNearestLight(int, int);
light *getLightViaId(unsigned int);
void deleteDynamicLight(light*);
void resetLight(light*);
void lightLogic(void);
void drawLights(void);
void drawDynamicLights(void);
void startLights(void);
void lightsShutdown(void);
void refreshAllLights(void);
int isPositionLit(int, int);
