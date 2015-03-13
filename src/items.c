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
itemCard *ITEM_CARDS = NULL;

void activateAllSeeingEye(item*);
void createAllItemCards(void);
item *getNewestItem(void);


void itemSetup() {
	ITEM_CONSOLE = TCOD_console_new(WINDOW_WIDTH, WINDOW_HEIGHT);

	TCOD_console_set_default_background(ITEM_CONSOLE, TCOD_color_RGB(255, 0, 255));
	TCOD_console_set_key_color(ITEM_CONSOLE, TCOD_color_RGB(255, 0, 255));
	
	createAllItemCards();
}

TCOD_console_t getItemConsole() {
	return ITEM_CONSOLE;
}

void createAllItemCards() {
	createItemCard(&createTreasure, RARITY_MEDIUM);
	createItemCard(&createKey, RARITY_KEY);
}

item *getItems() {
	return ITEMS;
}

item *getNewestItem() {
	item *_c = ITEMS;
	
	while (_c->next) {
		_c = _c->next;
	}
	
	return _c;
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
	_c->lodgedInActor = NULL;
	_c->foreColor = foreColor;
	_c->backColor = backColor;
	_c->statDamage = 0;
	_c->statSpeed = 0;
	
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

itemCard *createItemCard(void (*createItem)(int, int), int rarity) {
	itemCard *_c, *_p_c;
	
	_c = calloc(1, sizeof(itemCard));
	_c->createItem = createItem;
	_c->rarity = rarity;
	
	if (ITEM_CARDS == NULL) {
		ITEM_CARDS = _c;
	} else {
		_p_c = ITEM_CARDS;
		
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

	if (itm->owner || itm->lodgedInActor) {
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

item *getItemLodgedInActor(character *actor) {
	item *ptr = ITEMS;
	
	while (ptr) {
		if (ptr->lodgedInActor == actor) {
			return ptr;
		}
		
		ptr = ptr->next;
	}
	
	return NULL;
}

int getTotalNumberOfKeytorches() {
	int count = 0;
	item *ptr = ITEMS;
	
	while (ptr != NULL) {
		if (ptr->itemFlags & IS_KEYTORCH) {
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
		if (ptr->itemFlags & IS_KEYTORCH && ptr->itemLight->fuel) {
			count ++;
		}
		
		ptr = ptr->next;
	}

	return count;
}

//WARNING: If you delete an item, RETURN.
void itemHandleCharacterCollision(item *itm, character *actor) {
	character *player = getPlayer();
	
	if (itm->owner || itm->lodgedInActor) {
		return;
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
		
		if (itm->itemFlags & CAN_PICK_UP) {
			pickUpItem(actor, itm);
		}
	}
}

int itemHandleCharacterTouch(item *itm, character *actor) {
	character *player = getPlayer();
	
	if (actor == player) {
		if (itm->itemFlags & IS_DOOR) {
			if (itm->itemFlags & NEEDS_KEY && !actorGetItemWithFlag(player, IS_KEY)) {
				showMessage("%cThe door refuses to open...%c", 10);

				return 0;
			}
			unblockPosition(itm->x, itm->y);
			deleteItem(itm);
			
			return 1;
		}

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
		
		if (itm->itemFlags & IS_ALLSEEING_EYE) {
			activateAllSeeingEye(itm);
		}
	}
	
	return 0;
}


item *spawnItemWithRarity(int x, int y, int minRarity, int maxRarity) {
	int i, listLength = 0;
	itemCard *itemList[255];
	itemCard *itemCardPtr = ITEM_CARDS;
	
	while (itemCardPtr) {
		if (minRarity <= itemCardPtr->rarity <= maxRarity) {
			for (i = 0; i < itemCardPtr->rarity; i ++) {
				itemList[listLength] = itemCardPtr;
				listLength ++;
			}
		}
		
		itemCardPtr = itemCardPtr->next;
	}
	
	itemList[getRandomInt(0, listLength - 1)]->createItem(x, y);
	
	return getNewestItem();
}


//Item list

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
	item *itm = createItem(x, y, '!', TCOD_color_RGB(15, 15, 15), TCOD_color_RGB(55, 55, 55), IS_FUEL_SOURCE | IS_KEYTORCH);

	light *lght = createDynamicLight(x, y, NULL);
	itm->itemLight = lght;
	lght->r_tint = 5;
	lght->g_tint = 70;
	lght->b_tint = 0;
	lght->fuel = 0;
	lght->fuelMax = 280;
	lght->size = 5;

	blockPosition(itm->x, itm->y);
}

void createUnkindledBonfire(int x, int y) {
	item *itm = createItem(x, y, '!', TCOD_color_RGB(55, 55, 15), TCOD_color_RGB(55, 0, 55), IS_FUEL_SOURCE);
	light *lght = createDynamicLight(x, y, NULL);
	itm->itemLight = lght;
	lght->fuel = 0;
	lght->fuelMax = 120;

	blockPosition(itm->x, itm->y);
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

void createDoor(int x, int y) {
	createItem(x, y, '#', TCOD_color_RGB(50, 175, 175), TCOD_color_RGB(50, 75, 75), IS_DOOR | NEEDS_KEY);
}

void createKey(int x, int y) {
	createItem(x, y, '-', TCOD_color_RGB(30, 175, 175), TCOD_color_RGB(30, 75, 75), IS_KEY | CAN_PICK_UP);
}

void createWoodenSword(int x, int y) {
	item *itm = createItem(x, y, '/', TCOD_color_RGB(210, 105, 30), TCOD_color_RGB(30, 30, 30), IS_WEAPON | IS_SWORD | CAN_PICK_UP);
	
	itm->statDamage = 3;
	itm->statSpeed = 3;
}

void createTorchHolder(int x, int y) {
	createItem(x, y, 'U', TCOD_color_RGB(50, 50, 50), TCOD_color_RGB(10, 10, 10), IS_TORCH_HOLDER | CAN_PICK_UP);
}

void activateAllSeeingEye(item *itm) {
	int x, y;
	TCOD_console_t seenConsole = getSeenConsole();
	TCOD_map_t levelMap = getLevelMap();
	TCOD_map_t tunnelMap = getTunnelMap();
	
	for (y = 0; y < WINDOW_HEIGHT; y ++) {
		for (x = 0; x < WINDOW_WIDTH; x ++) {
			if (TCOD_map_is_walkable(levelMap, x, y) || TCOD_map_is_walkable(tunnelMap, x, y)) {
				drawCharBack(seenConsole, x, y, TCOD_color_RGB(255, 0, 255));
			}
		}
	}
	
	itm->itemLight->fuel = 10;
	itm->foreColor.r = 10;
	itm->foreColor.g = 10;
	//deleteDynamicLight(itm->itemLight);
	//itm->itemLight = NULL;
	itm->itemFlags ^= IS_ALLSEEING_EYE;
}

void createAllSeeingEye(int x, int y) {
	item *itm = createItem(x, y, ' ', TCOD_color_RGB(175, 175, 30), TCOD_color_RGB(75, 75, 0), IS_ALLSEEING_EYE);
	
	blockPosition(itm->x, itm->y);
	itm->chr = 207;
	light *lght = createDynamicLight(x, y, NULL);
	itm->itemLight = lght;
	lght->size = 3;
	lght->r_tint = 70;
	lght->g_tint = 70;
	lght->b_tint = 0;
	lght->fuel = 999999;
	lght->fuelMax = 999999;
}

void enableDoor(item *itm) {
	blockPosition(itm->x, itm->y);
	
	light *lght = createDynamicLight(itm->x, itm->y, NULL);
	itm->itemLight = lght;
	lght->r_tint = 50;
	lght->g_tint = 175;
	lght->b_tint = 175;
	lght->fuel = 99999;
	lght->fuelMax = 99999;
	lght->size = 3;
}
