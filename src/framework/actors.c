#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "../graphics.h"
#include "../level.h" //Will fix this later
#include "../lights.h"
#include "../items.h"
#include "../player.h"
#include "../combat.h"
#include "../ui.h"
#include "display.h"
#include "logging.h"
#include "actors.h"
#include "draw.h"
#include "../enemies.h"
#include "../entities.h"


TCOD_console_t ACTOR_CONSOLE;
static character *CHARACTERS = NULL;


void actorSetup() {
	ACTOR_CONSOLE = TCOD_console_new(WINDOW_WIDTH, WINDOW_HEIGHT);
}

void actorsShutdown() {
	character *next, *ptr = CHARACTERS;
	
	printf("Cleaning up actors...\n");
	
	while (ptr != NULL) {
		next = ptr->next;
		
		free(ptr);

		ptr = next;
	}
}

character *createActor(int x, int y) {
	character *_c, *_p_c;
	
	if (x <= 0 || x > WINDOW_WIDTH || y <= 0 || y > WINDOW_HEIGHT) {
		printf("Something has went terribly wrong!\n");

		if (x <= 0 || y <= 0) {
			printf("*FATAL* Actor placed OOB\n");

			assert(x > 0 && y > 0);
		}
	}
	
	_c = calloc(1, sizeof(character));
	_c->entityId = createEntity(getWorld());
	_c->x = x;
	_c->y = y;
	_c->lastX = x;
	_c->lastY = y;
	_c->hpMax = 10;
	_c->hp = _c->hpMax;
	_c->vx = 0;
	_c->vy = 0;
	_c->delay = 0;
	_c->statLevel = 1;
	_c->statLuck = 2;
	_c->statSpeed = 1;
	_c->statStrength = 3;
	_c->sightRange = 16;
	_c->numberOfItems = 0;
	_c->prev = NULL;
	_c->next = NULL;
	_c->itemLight = createDynamicLight(_c->x, _c->y, _c);
	_c->hp = 100;
	_c->fov = copyLevelMap();
	_c->path = TCOD_path_new_using_map(_c->fov, 1.41f);
	_c->chr = (int)'@';
	_c->foreColor = TCOD_color_RGB(255, 255 - RED_SHIFT, 255 - RED_SHIFT);
	_c->backColor = TCOD_color_RGB(255, 0, 0);
	_c->stanceFlags = IS_STANDING;
	_c->nextStanceFlagsToAdd = 0x0;
	_c->nextStanceFlagsToRemove = 0x0;
	
	if (CHARACTERS == NULL) {
		CHARACTERS = _c;
	} else {
		_p_c = CHARACTERS;
		
		while (_p_c->next) {
			_p_c = _p_c->next;
		}
		
		_p_c->next = _c;
		_c->prev = _p_c;
	}

	return _c;
}

void deleteActor(character *chr) {
	item *lodgedItem = getItemLodgedInActor(chr);
	
	if (chr == getPlayer()) {
		killPlayer();
	}

	deleteEntity(getWorld(), chr->entityId);
	
	if (lodgedItem) {
		lodgedItem->itemFlags ^= IS_LODGED;
		lodgedItem->lodgedInActor = NULL;
		lodgedItem->x = chr->x;
		lodgedItem->y = chr->y;
	}

	if (chr->itemLight) {
		deleteDynamicLight(chr->itemLight);
		chr->itemLight = NULL;
	}

	if (chr == CHARACTERS) {
		CHARACTERS = chr->next;
	} else {
		chr->prev->next = chr->next;

		if (chr->next) {
			chr->next->prev = chr->prev;
		}
	}

	free(chr);
}

void deleteEnemies() {
	character *next = NULL, *ptr = CHARACTERS;

	if (CHARACTERS == NULL) {
		printf("No enemies to delete.\n");

		return;
	}

	while (ptr != NULL) {
		next = ptr->next;

		if (ptr != getPlayer()) {
			deleteActor(ptr);
		}

		ptr = next;
	}
}

character *getActorViaId(unsigned int entityId) {
	character *ptr = CHARACTERS;

	while (ptr) {
		if (ptr->entityId == entityId) {
			return ptr;
		}

		ptr = ptr->next;
	}

	printf("*FATAL* Could not find entity ID=%i\n", entityId);

	return NULL;
}

void unsetStance(character *actor, unsigned int stance) {
	actor->stanceFlags ^= stance;
}

void setStance(character *actor, unsigned int stance) {
	actor->stanceFlags |= stance;
}

void setFutureStanceToAdd(character *actor, unsigned int stances) {
	actor->nextStanceFlagsToAdd |= stances;
}

void setFutureStanceToRemove(character *actor, unsigned int stances) {
	actor->nextStanceFlagsToRemove |= stances;
}

void setDelay(character *actor, int time) {
	actor->turns = 1;
	actor->delay = time;
}

int getMovementCost(character *actor) {
	int cost = actor->statSpeed;
	item *lodgedItem = getItemLodgedInActor(actor);
	
	if (actor->stanceFlags & IS_CRAWLING) {
		cost *= 2;
	}
	
	if (lodgedItem) {
		cost += lodgedItem->statDamage;
	}
	
	return cost;
}

int getActorLevel(character *actor) {
	return actor->statLevel;
}

int getActorLuck(character *actor) {
	return actor->statLuck;
}

int getActorStrength(character *actor) {
	return actor->statStrength;
}

int getActorSpeed(character *actor) {
	return actor->statSpeed;
}

void moveActor(character *actor, int vx, int vy) {
	if (actor->delay) {
		return;
	}
	
	actor->vx = vx;
	actor->vy = vy;
	
	setDelay(actor, getMovementCost(actor));
}

void walkActor(character *actor, int dx, int dy) {
	int currentDx = -1, currentDy = -1;
	
	if (!isPositionWalkable(dx, dy)) {
		printf("Invalid pathing destination: %i, %i\n", dx, dy);
		
		return;
	}
	
	if (TCOD_path_size(actor->path)) {
		TCOD_path_get_destination(actor->path, &currentDx, &currentDy);
		
		if (dx == currentDx && dy == currentDy) {
			printf("Existing path.\n");
			
			return;
		}
	}
	
	if (!TCOD_path_compute(actor->path, actor->x, actor->y, dx, dy)) {
		printf("Invalid path!\n");
	}
}

void walkActorPath(character *actor) {
	int nx, ny;
	
	if (TCOD_path_size(actor->path)) {
		TCOD_path_walk(actor->path, &nx, &ny, true);
		
		moveActor(actor, nx - actor->x, ny - actor->y);
	}
}

void removeItemFromInventory(character *actor, item *itm) {
	int i, removeItemAtIndex = -1;
	
	itm->owner = NULL;
	
	for (i = 0; i < actor->numberOfItems; i ++) {
		if (actor->inventory[i] == itm) {
			actor->inventory[i] = NULL;
			removeItemAtIndex = i;
		} else if (removeItemAtIndex != -1) {
			actor->inventory[i - 1] = actor->inventory[i];
		}
	}
	
	if (removeItemAtIndex == -1) {
		printf("*FATAL* Could not find item for removal.\n");
		
		return;
	}
	
	actor->numberOfItems --;
}

void pickUpItem(character *actor, item *itm) {
	//char itemPickupMessage[120];

	//strcpy(itemPickupMessage, "You pick up ");
	//strcat(itemPickupMessage, itm->name);
	char *itemPickupMessage = itm->name;
	itm->owner = actor;

	actor->inventory[actor->numberOfItems] = itm;
	actor->numberOfItems ++;
	
	if (actor == getPlayer()) {
		printf("%s\n", itemPickupMessage);
		showMessage(itemPickupMessage, 10);
	}
}

void dropItem(character *actor, item *itm) {
	itm->x = actor->x;
	itm->y = actor->y;
	
	removeItemFromInventory(actor, itm);
	//actor->inventory[actor->numberOfItems] = itm;
	//actor->numberOfItems ++;
	
	if (actor == getPlayer()) {
		showMessage("Dropped item.", 10);
	}
}

item *actorGetItemWithFlag(character *actor, unsigned int flag) {
	int i;

	for (i = 0; i < actor->numberOfItems; i ++) {
		if (actor->inventory[i]->itemFlags & flag) {
			return actor->inventory[i];
		}
	}

	return NULL;
}

void _resetActorForNewLevel(character *actor) {
	if (actor->fov) {
		TCOD_map_delete(actor->fov);
	}
	
	actor->fov = copyLevelMap();
	
	if (actor->path) {
		TCOD_path_delete(actor->path);
	}
	
	actor->path = TCOD_path_new_using_map(actor->fov, 1.41f);

	if (actor->itemLight) {
		resetLight(actor->itemLight);
	}

	TCOD_map_compute_fov(actor->fov, actor->x, actor->y, actor->sightRange, 1, FOV_SHADOW);
}

void resetAllActorsForNewLevel() {
	character *ptr = CHARACTERS;
	
	while (ptr != NULL) {
		_resetActorForNewLevel(ptr);
		
		ptr = ptr->next;
	}
}

void plantTorch(character *actor) {
	if (!actor->itemLight) {
		printf("Trying to plant a torch that isn't held.\n");

		return;
	}

	createPlantedTorch(actor->x, actor->y, actor->itemLight);

	actor->itemLight = NULL;
}

void _checkForItemCollisions(character *actor) {
	if (!actor) {
		printf("PANIC\n");
	}
	
	item *next, *ptr = getItems();

	while (ptr != NULL) {
		next = ptr->next;
		
		if (actor->x == ptr->x && actor->y == ptr->y) {
			itemHandleCharacterCollision(ptr, actor);
		}

		ptr = next;
	}
}

int _checkForTouchedItemAndHandle(character *actor, int x, int y) {
	item *next, *ptr = getItems();

	while (ptr != NULL) {
		if (ptr->owner) {
			ptr = ptr->next;

			continue;
		}

		next = ptr->next;
		
		if (x == ptr->x && y == ptr->y) {
			if (itemHandleCharacterTouch(ptr, actor)) {
				return 1;
			}
		}

		ptr = next;
	}
	
	return 0;
}

int _checkIfPositionLit(character *actor) {
	if (!isPositionLit(actor->x, actor->y)) {
		printf("Actor is in unlit position\n");

		if (actor == getPlayer()) {
			showMessage("You are lost to the shadows...", 20);
		}

		killActor(actor);

		return 1;
	}

	return 0;
}

void _actorAiTrack(character *actor) {
	if (!getPlayer()) {
		return;
	}

	walkActor(actor, getPlayer()->x, getPlayer()->y);
	walkActorPath(actor);
}

void _actorAi(character *actor) {
	if (actor->delay) {
		return;
	}
	
	if (actor->aiFlags & RANDOM_WALK) {
		moveActor(actor, getRandomInt(-1, 1), getRandomInt(-1, 1));
	} else if (actor->aiFlags & TRACK_TARGET) {
		_actorAiTrack(actor);
	} else if (actor->aiFlags & WORM_WALK) {
		//TODO: Potentially store AI info in a different struct,
		//a linked list containing AI states per AI type

		//TODO: Move to its own function
		character *player = getPlayer();

		if (!player) {
			return;
		}

		if (TCOD_map_is_in_fov(actor->fov, player->x, player->y)) {
			printf("Player is visible!\n");
		}
	}
}

void _actorLogic(character *actor) {
	if (!actor->hp) {
		return;
	}
	
	character *player = getPlayer(), *ptr = CHARACTERS;
	item *weaponPtr = NULL, *actorWeapon = actorGetItemWithFlag(actor, IS_WEAPON);
	int hitActor = 0;
	int nx = actor->x + actor->vx;
	int ny = actor->y + actor->vy;
	
	if (!actor->turns) {
		return;
	}
	
	if (actor->delay > 1) {
		actor->delay --;
		
		return;
	}
	
	actor->turns = 0;
	actor->delay = 0;

	if (actor->nextStanceFlagsToAdd) {
		actor->stanceFlags |= actor->nextStanceFlagsToAdd;
		
		actor->nextStanceFlagsToAdd = 0x0;
	}

	if (actor->nextStanceFlagsToRemove) {
		actor->stanceFlags ^= actor->nextStanceFlagsToRemove;
		
		if (actor->nextStanceFlagsToRemove & IS_STUNNED) {
			if (actor == player) {
				showMessage("You regain composure.", 5);
			} else {
				showMessage("It regains composure.", 5);
			}
		} else if (actor->nextStanceFlagsToRemove & IS_HOLDING_LODGED_WEAPON) {
			if (actor == player) {
				showMessage("You dislodge the weapon.", 5);
			} else {
				showMessage("Something dislodges their weapon.", 5);
			}
		}
		
		if (actor->nextStanceFlagsToRemove & IS_STUCK_WITH_LODGED_WEAPON) {
			weaponPtr = getItemLodgedInActor(actor);
			
			weaponPtr->itemFlags ^= IS_LODGED;
			weaponPtr->lodgedInActor = NULL;
		}
		
		actor->nextStanceFlagsToRemove = 0x0;
	}
	
	while (ptr != NULL) {
		if (ptr == actor || ptr->hp <= 0) {
			ptr = ptr->next;
			
			continue;
		}
		
		if (ptr->x == nx && ptr->y == ny) {
			hitActor = 1;
			
			break;
		}

		ptr = ptr->next;
	}
	
	if (_checkForTouchedItemAndHandle(actor, nx, ny)) {
		actor->vx = 0;
		actor->vy = 0;
		
		return;
	}

	if (hitActor) {
		if (attack(actor, ptr)) {
			return;
		}
		
		actor->vx = 0;
		actor->vy = 0;
	} else if (actor->vx || actor->vy) {
		if (actor->stanceFlags & IS_STUCK_WITH_LODGED_WEAPON && getItemLodgedInActor(actor)->owner) {
			printf("Cant move because of stuck weapon.\n");
			
			return;
		}
		
		if (actor->stanceFlags & IS_HOLDING_LODGED_WEAPON) {
			actor->stanceFlags ^= IS_HOLDING_LODGED_WEAPON;
			dropItem(actor, actorWeapon);
			
			printf("Handle letting go of weapon!\n");
			
			showMessage("%cYou let go of the weapon.%c", 10);
			
			return;
		}
		
		if (isPositionWalkable(nx, ny)) {
			if (actor->aiFlags & IS_VOID_WORM) {
				createVoidWormTail(actor->x, actor->y);
			}

			actor->lastX = actor->x;
			actor->lastY = actor->y;
			actor->x = nx;
			actor->y = ny;
			setStance(actor, IS_MOVING);

			if (actor->itemLight) {
				actor->itemLight->fuel -= getLevel();
			}

			TCOD_map_compute_fov(actor->fov, actor->x, actor->y, actor->sightRange, 1, FOV_SHADOW);
		} else {
			if (actor->stanceFlags & IS_MOVING) {
				unsetStance(actor, IS_MOVING);
			}
		}
	} else {
		if (actor->stanceFlags & IS_MOVING) {
			unsetStance(actor, IS_MOVING);
		}
	}

	_checkForItemCollisions(actor);
	if (!(actor->aiFlags & IS_IMMUNE_TO_DARKNESS) && _checkIfPositionLit(actor)) {
		return;
	}
	
	if (actor->itemLight) {
		actor->itemLight->x = actor->x;
		actor->itemLight->y = actor->y;
	}

	actor->vx = 0;
	actor->vy = 0;
}

void actorLogic() {
	character *next = NULL, *ptr = CHARACTERS;

	while (ptr != NULL) {
		next = ptr->next;

		_actorAi(ptr);
		_actorLogic(ptr);

		ptr = next;
	}
}

void _drawActor(character *actor) {
	if (actor->hp <= 0 || (isTransitionInProgress() && actor->itemLight && actor->itemLight->sizeMod == 0)) {
		return;
	}
	
	TCOD_color_t foreColor = actor->foreColor;
	int nx, ny, occupiedPosition = 0, chr = actor->chr;
	float healthPercentage = (float)actor->hp / (float)actor->hpMax;
	character *actorPtr = getActors();
	
	if (getAnimateFrame() / 60.f >= .5) {
		if (actor->stanceFlags & IS_STUCK_WITH_LODGED_WEAPON) {
			foreColor.r = 255;
			foreColor.g = 0;
			foreColor.b = 0;
			
			chr = getItemLodgedInActor(actor)->chr;
		} else if (actor->stanceFlags & IS_CRAWLING && actor->stanceFlags & IS_STUNNED) {
			chr = 25;
		} else if (actor->stanceFlags & IS_STUNNED) {
			chr = (int)'*';
		} else if (actor->stanceFlags & IS_CASTING) {
			chr = (int)'!';
		}
	}
	
	if (healthPercentage <= .75) {
		if ((getAnimateFrame() % (int)(60 * ((healthPercentage / .75)))) == 0) {
			foreColor.r = 255;
			foreColor.g = 0;
			foreColor.b = 0;
		}
	}
	
	drawChar(ACTOR_CONSOLE, actor->x, actor->y, chr, foreColor, actor->backColor);
	
	if (actor->vx || actor->vy) {
		nx = actor->x + actor->vx;
		ny = actor->y + actor->vy;
		
		while (actorPtr) {
			if (actorPtr->x == nx && actorPtr->y == ny) {
				occupiedPosition = 1;
				
				break;
			}
			
			actorPtr = actorPtr->next;
		}
		
		if (!occupiedPosition || getAnimateFrame() / 60.f >= .5) {
			drawChar(ACTOR_CONSOLE, nx, ny, 176, foreColor, actor->backColor);
		}
	}
}

void drawActors() {
	character *ptr = CHARACTERS;

	while (ptr != NULL) {
		_drawActor(ptr);

		ptr = ptr->next;
	}
}

void killActor(character *actor) {
	TCOD_console_t levelConsole = getLevelConsole();
	
	actor->hp = 0;
	if (actor->itemLight) {
		if (actor->aiFlags & DROP_LIGHT_ON_DEATH) {
			actor->itemLight->owner = NULL;

			if (actor->itemLight->fuel > 25) {
				actor->itemLight->fuel = 25;
			}
		} else {
			deleteDynamicLight(actor->itemLight);
		}
		
		actor->itemLight = NULL;
	}
	
	drawChar(levelConsole, actor->x, actor->y, actor->chr, TCOD_color_RGB(actor->foreColor.r * .55f, actor->foreColor.g * .55f, actor->foreColor.b * .55f), actor->backColor);

	printf("Killed actor.\n");
	
	if (actor == getPlayer()) {
		showMessage("You die.", 15);
	}
}

void actorCleanup() {
	character *nextChar, *ptr = CHARACTERS;
	int freedActors = 0;

	while (ptr != NULL) {
		nextChar = ptr->next;
		
		if (ptr->hp <= 0) {
			deleteActor(ptr);

			freedActors ++;
		}

		ptr = nextChar;
	}

	if (freedActors) {
		printf("%i actor(s) freed.\n", freedActors);
	}

	TCOD_console_clear(ACTOR_CONSOLE);
}

character *getActors() {
	return CHARACTERS;
}

TCOD_console_t getActorConsole() {
	return ACTOR_CONSOLE;
}
