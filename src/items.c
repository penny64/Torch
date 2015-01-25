#include "libtcod.h"
#include "framework/display.h"
#include "framework/actors.h"
#include "framework/draw.h"
#include "items.h"
#include "lights.h"
#include "player.h"
#include "ui.h"


TCOD_console_t ITEM_CONSOLE;
item *ITEMS = NULL;


void itemSetup() {
	ITEM_CONSOLE = TCOD_console_new(WINDOW_WIDTH, WINDOW_HEIGHT);

	TCOD_console_set_default_background(ITEM_CONSOLE, TCOD_color_RGB(255, 0, 255));
	TCOD_console_set_key_color(ITEM_CONSOLE, TCOD_color_RGB(255, 0, 255));
}

TCOD_console_t getItemConsole() {
	return ITEM_CONSOLE;
}

item *getItems() {
	return ITEMS;
}

item *createItem(int x, int y, char chr, unsigned int flags) {
	item *_c, *_p_c;
	
	_c = calloc(1, sizeof(item));
	_c->x = x;
	_c->y = y;
	_c->chr = (int)chr;
	_c->itemFlags = flags;
	_c->owner = NULL;
	_c->prev = NULL;
	_c->next = NULL;
	
	if (ITEMS == NULL) {
		ITEMS = _c;
	} else {
		_p_c = ITEMS;
		
		while (_p_c->next) {
			_p_c = _p_c->next;
		}
		
		_p_c->next = _c;
		_c->prev = _p_c;
	}

	return _c;
}

void assignFlag(item *itm, unsigned int flag) {
	itm->itemFlags |= flag;
}

void _itemLogic(item *itm) {
}

void _drawItem(item *itm) {
	TCOD_console_t itemConsole = getItemConsole();

	if (itm->owner) {
		return;
	}

	drawChar(itemConsole, itm->x, itm->y, (int)'i', TCOD_color_RGB(255, 255, 155), TCOD_color_RGB(0, 0, 0));
}

void resetItemForNewLevel(item *itm) {
}

void itemLogic() {
	item *ptr = ITEMS;
	
	while (ptr != NULL) {
		_itemLogic(ptr);
		
		ptr = ptr->next;
	}
}

void drawItems() {
	item *ptr = ITEMS;
	TCOD_console_t itemConsole = getItemConsole();

	TCOD_console_clear(itemConsole);
	
	while (ptr != NULL) {
		_drawItem(ptr);
		
		ptr = ptr->next;
	}	
}

void itemHandleCharacterCollision(item *itm, character *actor) {
	character *player = getPlayer();

	if (itm->itemFlags & IS_FUEL_SOURCE) {
		actor->itemLight->fuel = actor->itemLight->fuelMax;

		if (actor == player) {
			if (itm->itemLight && !itm->itemLight->fuel) {
				itm->itemLight->fuel = itm->itemLight->fuelMax;
				
				showMessage("%cBonfire rekindled. Torch has x fuel remaining.%c", 10);
			} else {
				showMessage("%cTorch rekindled. Bonfire has x fuel remaining.%c", 10);
			}
		}
	}
}

void createBonfire(int x, int y) {
	item *itm = createItem(x, y, '!', IS_FUEL_SOURCE | IS_SOMETHING);

	light *lght = createDynamicLight(x, y, NULL);
	itm->itemLight = lght;
	lght->fuel = 180;
	lght->fuelMax = 180;
}

void createBonfireKeystone(int x, int y) {
	item *itm = createItem(x, y, '!', IS_FUEL_SOURCE | IS_SOMETHING | IS_KEY);

	light *lght = createDynamicLight(x, y, NULL);
	itm->itemLight = lght;
	lght->r_tint = 15;
	lght->g_tint = 80;
	lght->b_tint = 5;
	lght->fuel = 0;
	lght->fuelMax = 280;
}

void createUnkindledBonfire(int x, int y) {
	item *itm = createItem(x, y, '!', IS_FUEL_SOURCE | IS_SOMETHING);

	light *lght = createDynamicLight(x, y, NULL);
	itm->itemLight = lght;
	lght->fuel = 0;
	lght->fuelMax = 120;
}
