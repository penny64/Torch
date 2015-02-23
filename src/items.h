#include "libtcod.h"

#ifndef ITEMS_H
#define ITEMS_H
#include "framework/actors.h"
#include "lights.h"


enum {
	IS_FUEL_SOURCE = 0x01 << 0,
	IS_TORCH = 0x01 << 1,
	IS_KEYTORCH = 0x01 << 2,
	IS_EXIT = 0x01 << 3,
	IS_SINGLE_USE_FUEL_SOURCE = 0x01 << 4,
	IS_DOOR = 0x01 << 5,
	NEEDS_KEY = 0x01 << 6,
	IS_KEY = 0x01 << 7,
	CAN_PICK_UP = 0x01 << 8,
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
void deleteItem(item*);
void assignFlag(item*, unsigned int);
void itemHandleCharacterCollision(item*, character*);
int itemHandleCharacterTouch(item*, character*);
TCOD_console_t getItemConsole(void);
item *getItems(void);
void drawItems(void);
int getTotalNumberOfKeytorches(void);
int getNumberOfLitKeytorches(void);
void deleteAllOwnerlessItems(void);

//Item list
void createBonfire(int, int);
void createBonfireKeystone(int, int);
void createUnkindledBonfire(int, int);
void createTreasure(int, int);
void createPlantedTorch(int, int, light*);
void createExit(int, int);
void createSign(int, int, char*);
void createDoor(int, int);
void createKey(int, int);
void enableDoor(item*);

#endif