#include "libtcod.h"
#include "framework/actors.h"
#include "lights.h"


enum {
	IS_FUEL_SOURCE = 0x01,
	IS_TORCH = 0x02,
	IS_KEY = 0x04,
	IS_SOMETHING_SOMETHING_ELSE = 0x08,
} itemFlag_t;


typedef struct item item;

struct item {
	int x, y, vx, vy, chr;
	unsigned int itemFlags;
	struct item *next, *prev;
	struct character *owner;
	struct light *itemLight;
	TCOD_color_t foreColor, backColor;
};


void itemSetup(void);
void itemsShutdown(void);
void itemLogic(void);
item *createItem(int, int, char, TCOD_color_t, TCOD_color_t, unsigned int);
void assignFlag(item*, unsigned int);
void itemHandleCharacterCollision(item*, character*);
TCOD_console_t getItemConsole(void);
item *getItems(void);
void drawItems(void);
int getTotalNumberOfKeytorches(void);
int getNumberOfLitKeytorches(void);

//Item list
void createBonfire(int, int);
void createBonfireKeystone(int, int);
void createUnkindledBonfire(int, int);
void createTreasure(int, int);
void createPlantedTorch(int, int, light*);
