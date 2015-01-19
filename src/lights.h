typedef struct light light;

struct light {
	int x, y;
	struct light *next, *prev;
	TCOD_console_t fov;
};


TCOD_map_t getLightMap(void);
light *createLight(int, int);
light *getLights(void);
void drawLights(void);
void startLights(void);
