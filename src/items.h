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
	IS_WEAPON = 0x01 << 9,
	IS_SWORD = 0x01 << 10,
	IS_LODGED = 0x01 << 11,
	IS_TORCH_HOLDER = 0x01 << 12,
	IS_ALLSEEING_EYE = 0x01 << 13,
	IGNORE_ALLSEEING_EYE = 0x01 << 14,
	IS_SOLID = 0x01 << 15,
	IS_DESTROYABLE = 0x01 << 16,
	IS_ARMOR = 0x01 << 17,
	ARE_BOOTS = 0x01 << 18,
	IS_DAGGER = 0x01 << 19,
	IS_HELPER_ITEM = 0x01 << 20,
	IS_CHEST_ARMOR = 0x01 << 21,
	IS_AMULET = 0x01 << 22,
	ARE_GAUNTLETS = 0x01 << 23,
	IS_HELMET = 0x01 << 24,
} itemFlag_t;

enum {
	IS_NORMAL = 0,
	IS_FLAMING = 0x01 << 0,
	IS_QUICK = 0x01 << 1,
	IS_LIGHTNING = 0x01 << 2,
	IS_FORCE = 0x01 << 3,
} itemEffectFlag_t;

enum {
	RARITY_NONE = 0,
	RARITY_LOW = 1,
	RARITY_MEDIUM = 2,
	RARITY_HIGH = 3,
	RARITY_KEY = 4,
} itemRarityFlag_t;


typedef struct item item;

struct item {
	int x, y, vx, vy, chr, statDamage, statStrength, statDefense, statSpeed, statLevel, statLuck;
	float statStability;
	unsigned int itemFlags, itemEffectFlags;
	char *name;
	struct item *next, *prev;
	struct character *owner, *lodgedInActor;
	struct light *itemLight;
	TCOD_color_t foreColor, backColor;
};

typedef struct itemCard itemCard;

struct itemCard {
	unsigned int rarity, itemFlags;
	struct itemCard *next, *prev;
	void (*createItem)(int, int);
};

void itemSetup(void);
void itemsShutdown(void);
void itemLogic(void);
item *createItem(int, int, char, TCOD_color_t, TCOD_color_t, unsigned int);
itemCard *createItemCard(void (*)(int, int), unsigned int, unsigned int);
void deleteItem(item*);
void assignFlag(item*, unsigned int);
int itemHandleCharacterCollision(item*, character*);
int itemHandleCharacterTouch(item*, character*);
TCOD_console_t getItemConsole(void);
item *getItems(void);
item *getNewestItem(void);
item *getItemLodgedInActor(character*);
item *spawnItemWithRarity(int, int, unsigned int, unsigned int, unsigned int);
void drawItems(void);
int getItemLevel(item*);
int getAttackSpeedOfWeapon(item*);
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
void createTorchHolder(int, int);
void createLowSword(int, int);
void createLowDagger(int, int);
void createBoots(int, int);
void createGauntlets(int, int);
void createChestArmor(int, int);
void createHelmet(int, int);
void createAmulet(int, int);
void createWoodWall(int, int);
void createMetalWall(int, int);
void createStoneWall(int, int);
void createAllSeeingEye(int, int);
void enableDoor(item*);
void enableSolid(item*);

#endif
