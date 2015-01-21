#include "libtcod.h"
#include "framework/actors.h"


enum {
  IS_FUEL_SOURCE = 0x01,
  IS_SOMETHING = 0x02,
  IS_SOMETHING_ELSE = 0x04,
  IS_SOMETHING_SOMETHING_ELSE = 0x08,
} itemFlag_t;


typedef struct item item;

struct item {
	int x, y, vx, vy, chr;
	unsigned int itemFlags;
	struct item *next, *prev;
};


void itemSetup(void);
void itemLogic(void);
item *createItem(int, int, char, unsigned int);
void assignFlag(item*, unsigned int);
void itemHandleCharacterCollision(item*, character*);
TCOD_console_t getItemConsole(void);
item *getItems(void);

//Item list
void createBonfire(int, int);