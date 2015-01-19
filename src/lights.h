typedef struct light light;

struct light {
	int x, y;
	struct light *next, *prev;
	TCOD_console_t fov;
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
