#include "framework/actors.h"

#ifndef LIGHTS_H
#define LIGHTS_H

typedef struct light light;

struct light {
	int x, y, fuel, fuelMax, size, r_tint, g_tint, b_tint;
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
void deleteDynamicLight(light*);
void resetLight(light*);
void lightLogic(void);
void drawLights(void);
void drawDynamicLights(void);
void startLights(void);
void lightsShutdown(void);
void refreshAllLights(void);
int isPositionLit(int, int);
