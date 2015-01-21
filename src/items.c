#include "libtcod.h"
#include "framework/display.h"
#include "framework/actors.h"
#include "items.h"
#include "lights.h"


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
	
	_c = malloc(sizeof(item));
	_c->x = x;
	_c->y = y;
	_c->chr = (int)chr;
	_c->itemFlags = flags;
	_c->prev = NULL;
	_c->next = NULL;
	
	if (ITEMS == NULL) {
		ITEMS = _c;
	} else {
		_p_c = ITEMS;
		
		ITEMS->next = _c;
		_c->prev = _p_c;
	}

	return _c;
}

void assignFlag(item *itm, unsigned int flag) {
	itm->itemFlags |= flag;
}

void _itemLogic(item *itm) {
}

void itemLogic() {
	item *ptr = ITEMS;
	
	while (ptr != NULL) {
		_itemLogic(ptr);
		
		ptr = ptr->next;
	}
}

void itemHandleCharacterCollision(item *itm, character *actor) {
	if (itm->itemFlags & IS_FUEL_SOURCE) {
		actor->itemLight->fuel = actor->itemLight->fuelMax;
	}
}

void createBonfire(int x, int y) {
	createItem(x, y, '!', IS_FUEL_SOURCE | IS_SOMETHING);
}