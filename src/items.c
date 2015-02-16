#include <stdio.h>

#include "libtcod.h"
#include "framework/display.h"
#include "framework/actors.h"
#include "framework/draw.h"
#include "items.h"
#include "lights.h"
#include "player.h"
#include "ui.h"
#include "level.h"


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

item *createItem(int x, int y, char chr, TCOD_color_t foreColor, TCOD_color_t backColor, unsigned int flags) {
	item *_c, *_p_c;
	
	_c = calloc(1, sizeof(item));
	_c->x = x;
	_c->y = y;
	_c->chr = (int)chr;
	_c->itemFlags = flags;
	_c->owner = NULL;
	_c->prev = NULL;
	_c->next = NULL;
	_c->foreColor = foreColor;
	_c->backColor = backColor;
	
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

void itemsShutdown() {
	item *next, *ptr = ITEMS;
	
	printf("Shutting down items...\n");
	
	while (ptr != NULL) {
		next = ptr->next;
		
		free(ptr);
		
		ptr = next;
	}
}

void deleteAllOwnerlessItems() {
	item *next, *ptr = ITEMS;

	printf("Deleting all ownerless items...\n");

	while (ptr != NULL) {
		next = ptr->next;

		if (!ptr->owner) {
			deleteItem(ptr);
		}

		ptr = next;
	}
}

void deleteItem(item *itm) {
	if (itm->itemLight) {
		printf("Deleting item\n");
		deleteDynamicLight(itm->itemLight);
	}
	
	if (itm == ITEMS) {
		ITEMS = itm->next;
	} else {
		itm->prev->next = itm->next;

		if (itm->next) {
			itm->next->prev = itm->prev;
		}
	}

	free(itm);
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

	drawChar(itemConsole, itm->x, itm->y, (int)itm->chr, itm->foreColor, itm->backColor);
}

void resetItemForNewLevel(item *itm) {
}

void itemLogic() {
	item *ptr = ITEMS;
	
	while (ptr != NULL) {
		_itemLogic(ptr);
		
		ptr = ptr->next;
	}

	if (getTotalNumberOfKeytorches() > 0 && !isLevelComplete() && getNumberOfLitKeytorches() == getTotalNumberOfKeytorches()) {
		completeLevel();
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

int getTotalNumberOfKeytorches() {
	int count = 0;
	item *ptr = ITEMS;
	
	while (ptr != NULL) {
		if (ptr->itemFlags & IS_KEY) {
			count ++;
		}
		
		ptr = ptr->next;
	}

	return count;
}

int getNumberOfLitKeytorches() {
	int count = 0;
	item *ptr = ITEMS;
	
	while (ptr != NULL) {
		if (ptr->itemFlags & IS_KEY && ptr->itemLight->fuel) {
			count ++;
		}
		
		ptr = ptr->next;
	}

	return count;
}

//WARNING: If you delete an item, RETURN.
void itemHandleCharacterCollision(item *itm, character *actor) {
	character *player = getPlayer();

	if (itm->itemFlags & IS_FUEL_SOURCE && actor->itemLight) {
		actor->itemLight->fuel = actor->itemLight->fuelMax;

		if (actor == player) {
			if (itm->itemLight && actor->itemLight && actor->itemLight->fuel) {
				if (!itm->itemLight->fuel) {
					showMessage("%cBonfire rekindled. Torch has x fuel remaining.%c", 10);
				} else {
					showMessage("%cTorch rekindled. Bonfire has x fuel remaining.%c", 10);
				}

				itm->itemLight->fuel = itm->itemLight->fuelMax;
				itm->foreColor = TCOD_color_RGB(255, 255, 155);
				itm->backColor = TCOD_color_RGB(155, 155, 155);
			}
		}
	}

	if (actor->itemLight && itm->itemFlags & IS_SINGLE_USE_FUEL_SOURCE && itm->itemLight) {
		if (actor == player) {
			actor->itemLight->fuel = actor->itemLight->fuelMax;
			itm->foreColor = TCOD_color_RGB(55, 55, 15);
			itm->backColor = TCOD_color_RGB(25, 25, 25);

			deleteDynamicLight(itm->itemLight);
			itm->itemLight = NULL;
			
			printf("One of those\n");
		}
	}

	if (actor == player) {
		if (itm->itemFlags & IS_TORCH && !player->itemLight) {
			player->itemLight = itm->itemLight;
			itm->itemLight->owner = player;
			itm->itemLight = NULL;

			deleteItem(itm);

			showMessage("%cYou pick up the torch.%c", 10);
			
			return;
		}

		if (itm->itemFlags & IS_EXIT && isLevelComplete()) {
			if (!player->itemLight) {
				showMessage("%cYou forgot your torch!%c", 10);
			} else {
				exitLevel();
			}
		}
	}
}

void createBonfire(int x, int y) {
	item *itm = createItem(x, y, '!', TCOD_color_RGB(255, 255, 155), TCOD_color_RGB(55, 0, 55), 0x0);

	light *lght = createDynamicLight(x, y, NULL);
	itm->itemLight = lght;
	lght->fuel = 180;
	lght->fuelMax = 180;
	lght->size = 5;
	lght->r_tint = 65;
	lght->g_tint = 60;
	lght->b_tint = 65;
}

void createBonfireKeystone(int x, int y) {
	item *itm = createItem(x, y, '!', TCOD_color_RGB(15, 15, 15), TCOD_color_RGB(55, 55, 55), IS_FUEL_SOURCE | IS_KEY);

	light *lght = createDynamicLight(x, y, NULL);
	itm->itemLight = lght;
	lght->r_tint = 15;
	lght->g_tint = 80;
	lght->b_tint = 5;
	lght->fuel = 0;
	lght->fuelMax = 280;
	lght->size = 7;
}

void createUnkindledBonfire(int x, int y) {
	item *itm = createItem(x, y, '!', TCOD_color_RGB(55, 55, 15), TCOD_color_RGB(55, 0, 55), IS_FUEL_SOURCE);
	light *lght = createDynamicLight(x, y, NULL);
	itm->itemLight = lght;
	lght->fuel = 0;
	lght->fuelMax = 120;
}

void createPlantedTorch(int x, int y, light *lght) {
	lght->owner = NULL;
	lght->x = x;
	lght->y = y;

	item *itm = createItem(x, y, 'i', TCOD_color_RGB(255, 255, 155), TCOD_color_RGB(55, 0, 55), IS_TORCH);
	itm->itemLight = lght;
}

void createTreasure(int x, int y) {
	createItem(x, y, '*', TCOD_color_RGB(255, 255, 0), TCOD_color_RGB(55, 0, 55), 0x0);
}

void createSign(int x, int y, char *text) {
	createItem(x, y, 'D', TCOD_color_RGB(255, 255, 0), TCOD_color_RGB(55, 0, 55), 0x0);
}

void createExit(int x, int y) {
	createItem(x, y, '>', TCOD_color_RGB(200, 200, 150), TCOD_color_RGB(50, 50, 25), IS_EXIT);
}
