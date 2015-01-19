typedef struct light light;

struct light {
	int x, y, fuel, fuelMax, size;
	struct light *next, *prev;
	TCOD_map_t fov;
	TCOD_map_t lightMap;
};


TCOD_map_t getLightMap(void);
TCOD_console_t getDynamicLightConsole(void);
light *createLight(int, int);
light *createDynamicLight(int, int);
light *getLights(void);
light *getDynamicLights(void);
void lightLogic(void);
void drawLights(void);
void drawDynamicLights(void);
void startLights(void);
