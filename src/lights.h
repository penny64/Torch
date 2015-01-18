typedef struct light light;

struct light {
	int x, y;
	struct light *next, *prev;
	TCOD_console_t fov;
};


light *createLight(int, int);
light *getLights(void);
void drawLights(void);