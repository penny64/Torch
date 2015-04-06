#include <stdio.h>
#include <assert.h>

#include "framework/actors.h"
#include "framework/display.h"
#include "framework/numbers.h"
#include "framework/input.h"
#include "spells.h"
#include "systems.h"
#include "particles.h"
#include "entities.h"
#include "components.h"
#include "effects.h"
#include "level.h"
#include "lights.h"
#include "ui.h"


void createCastingMenu(World*, unsigned int);
void spellHandler(World*, unsigned int);
void spellTickHandler(World*, unsigned int);
void spellInputHandler(World*, unsigned int);
void spellCollisionWithSolidHandler(World*, unsigned int);
void spellCollisionWithActorHandler(World*, unsigned int);
void spellDeleteHandler(World*, unsigned int);
void spellMovementHandler(World*, unsigned int);
void spellAddStanceHandler(World*, unsigned int);
void spellRemoveStanceHandler(World*, unsigned int);
void fireball(World*, unsigned int);
void fireballTick(World*, unsigned int);
void fireballExit(World*, unsigned int);
void fireballHitSolid(World*, unsigned int);
void fireballHitActor(World*, unsigned int);
void _spellTargetCursorCallback(int, int);
void createTargetCursor(World*, unsigned int, void (*)(int, int));

Spell SPELL_FIREBALL = {"Fireball", &fireball, &fireballTick, &fireballExit, &fireballHitSolid, &fireballHitActor, SPELL_IS_FLAME | SPELL_IS_AIMABLE | SPELL_CAST_ON_EXIT, -1, DELAY_SHORT};

int UI_OWNER_ID;
World *UI_WORLD_PTR;


void startSpells() {
	World *world = getWorld();

	//createSystemHandler(world, COMPONENT_SPELL, 0x0, &spellHandler);
	createSystemHandler(world, EVENT_INPUT, COMPONENT_SPELL | COMPONENT_PLAYER, &spellInputHandler);
	createSystemHandler(world, EVENT_COLLISION_SOLID, COMPONENT_SPELL_BULLET, &spellCollisionWithSolidHandler);
	createSystemHandler(world, EVENT_COLLISION_ACTOR, COMPONENT_SPELL_BULLET, &spellCollisionWithActorHandler);
	createSystemHandler(world, EVENT_MOVED, COMPONENT_SPELL_BULLET, &spellMovementHandler);
	createSystemHandler(world, EVENT_DELETED, COMPONENT_SPELL_BULLET, &spellDeleteHandler);
	createSystemHandler(world, EVENT_ADD_STANCE, COMPONENT_SPELL, &spellAddStanceHandler);
	createSystemHandler(world, EVENT_TICK, COMPONENT_SPELL, &spellTickHandler);
	createSystemHandler(world, EVENT_REMOVE_STANCE, COMPONENT_SPELL, &spellRemoveStanceHandler);
}

void registerSpellSystem(World *world, unsigned int entityId) {
	world->mask[entityId] |= COMPONENT_SPELL;

	SpellComponent *spellComponent = &world->spell[entityId];
	spellComponent->activeSpell = -1;

	printf("Added entity ID=%i to system `spells`.\n", entityId);
}

void addSpell(World *world, unsigned int entityId, Spell spell) {
	SpellComponent *spellComponent = &world->spell[entityId];

	spellComponent->name[spellComponent->spellCount] = spell.name;
	spellComponent->castSpell[spellComponent->spellCount] = spell.castSpell;
	spellComponent->tickSpell[spellComponent->spellCount] = spell.tickSpell;
	spellComponent->exitSpell[spellComponent->spellCount] = spell.exitSpell;
	spellComponent->hitSolidSpell[spellComponent->spellCount] = spell.hitSolidSpell;
	spellComponent->hitActorSpell[spellComponent->spellCount] = spell.hitActorSpell;
	spellComponent->spellTraits[spellComponent->spellCount] = spell.spellMask;
	spellComponent->castDelay[spellComponent->spellCount] = spell.castDelay;

	spellComponent->spellCount ++;
}

void spellHandler(World *world, unsigned int entityId) {
	//SpellComponent *spellComponent = &world->spell[entityId];

}

void spellTickHandler(World *world, unsigned int entityId) {
	character *owner = getActorViaId(entityId);
	SpellComponent *spellComponent = &world->spell[entityId];

	if (owner->stanceFlags & IS_CASTING && spellComponent->tickSpell[spellComponent->activeSpell] != NULL) {
		spellComponent->tickSpell[spellComponent->activeSpell](world, entityId);
	}

}

void spellInputHandler(World *world, unsigned int entityId) {
	//SpellComponent *spellComponent = &world->spell[entityId];

	if (isCharPressed('x')) {
		castSpell(world, entityId);
	} else if (isCharPressed('X')) {
		createCastingMenu(world, entityId);
	}
}

void spellCollisionWithSolidHandler(World *world, unsigned int entityId) {
	RectComponent *rectComponent = &world->rect[entityId];
	SpellComponent *spellComponent = &world->spell[rectComponent->ownerId];

	if (spellComponent->hitSolidSpell[spellComponent->activeSpell]) {
		spellComponent->hitSolidSpell[spellComponent->activeSpell](world, entityId);
	}

	deleteEntity(world, entityId);
}

void spellCollisionWithActorHandler(World *world, unsigned int entityId) {
	RectComponent *rectComponent = &world->rect[entityId];
	SpellComponent *spellComponent = &world->spell[rectComponent->ownerId];
	//character *owner = getActorViaId(rectComponent->ownerId);

	printf("Hereee\n");
	if (spellComponent->hitActorSpell[spellComponent->activeSpell]) {
		spellComponent->hitActorSpell[spellComponent->activeSpell](world, entityId);
	}
	//character *target = getActorViaId((unsigned int)rectComponent->collidingWithEntityId);

	//target->hp -= 100;

	deleteEntity(world, entityId);
}

void spellMovementHandler(World *world, unsigned int entityId) {
	RectComponent *rectComponent = &world->rect[entityId];
	LightComponent *lightComponent = &world->light[entityId];

	light *lightPtr = getLightViaId(lightComponent->lightId);

	if (!lightPtr) {
		return;
	}

	lightPtr->x = rectComponent->x;
	lightPtr->y = rectComponent->y;

	//createParticle(rectComponent->x, rectComponent->y, 176, 0, 0, 1.f, .85, TCOD_color_RGB(128, 0, 0), TCOD_color_RGB(228, 0, 0));
}

void spellDeleteHandler(World *world, unsigned int entityId) {
	LightComponent *lightComponent = &world->light[entityId];

	light *lightPtr = getLightViaId(lightComponent->lightId);

	if (lightComponent->lightId != - 1 && !lightPtr) {
		return;
	}

	deleteDynamicLight(lightPtr);
	lightComponent->lightId = -1;
}

void spellAddStanceHandler(World *world, unsigned int entityId) {

}

void spellRemoveStanceHandler(World *world, unsigned int entityId) {
	character *owner = getActorViaId(entityId);
	SpellComponent *spellComponent = &world->spell[entityId];

	if (owner->nextStanceFlagsToRemove & IS_CASTING && spellComponent->exitSpell[spellComponent->activeSpell]) {
		spellComponent->exitSpell[spellComponent->activeSpell](world, entityId);
	}
}

void castSpell(World *world, unsigned int entityId) {
	SpellComponent *spellComponent = &world->spell[entityId];
	unsigned int owner = entityId;

	if (!spellComponent->spellCount) {
		showMessage(10, "No spells known.", NULL);

		return;
	}

	if (spellComponent->activeSpell == -1) {
		createCastingMenu(world, entityId);

		showMessage(10, "No spell selected.", NULL);

		return;
	}

	if (spellComponent->spellTraits[spellComponent->activeSpell] & SPELL_IS_AIMABLE) {
		createTargetCursor(world, entityId, &_spellTargetCursorCallback);
	} else {
		spellComponent->castSpell[spellComponent->activeSpell](world, owner);
	}
}

void _spellTargetCursorCallback(int x, int y) {
	character *owner = getActorViaId((unsigned int)UI_OWNER_ID);
	SpellComponent *spellComponent = &UI_WORLD_PTR->spell[UI_OWNER_ID];

	spellComponent->targetX = x;
	spellComponent->targetY = y;
	spellComponent->castSpell[spellComponent->activeSpell](UI_WORLD_PTR, owner->entityId);

	UI_WORLD_PTR = NULL;
	UI_OWNER_ID = -1;
}

void _castingMenuCallback(int menuItemIndex, char *menuItemString) {
	SpellComponent *spellComponent = &UI_WORLD_PTR->spell[UI_OWNER_ID];

	spellComponent->activeSpell = menuItemIndex;

	UI_WORLD_PTR = NULL;
	UI_OWNER_ID = -1;
}

void createTargetCursor(World *world, unsigned int entityId, void (*callback)(int, int)) {
	character *entity = getActorViaId(entityId);

	UI_OWNER_ID = entityId;
	UI_WORLD_PTR = world;

	createCursor(entity->x, entity->y, entity->fov, callback);
}

void createCastingMenu(World *world, unsigned int entityId) {
	SpellComponent *spellComponent = &world->spell[entityId];
	int i;
	char *menuStrings[WINDOW_HEIGHT];

	for (i = 0; i < spellComponent->spellCount; i ++) {
		menuStrings[i] = spellComponent->name[i];
	}

	menuStrings[i] = NULL;

	UI_OWNER_ID = entityId;
	UI_WORLD_PTR = world;

	createMenu(menuStrings, &_castingMenuCallback);
}

//Spells

void fireball(World *world, unsigned int ownerId) {
	character *owner = getActorViaId(ownerId);
	SpellComponent *spellComponent = &world->spell[ownerId];
	int x, y, spellDelay;

	x = owner->x;
	y = owner->y;
	spellDelay = spellComponent->castDelay[spellComponent->activeSpell];

	light *lght = createDynamicLight(x, y, owner);
	lght->r_tint = 255;
	lght->g_tint = 40;
	lght->b_tint = 40;
	lght->fuel = spellDelay;
	lght->fuelMax = spellDelay;

	setStance(owner, IS_CASTING);
	setFutureStanceToRemove(owner, IS_CASTING);
	setDelay(owner, spellDelay);
}

void fireballTick(World *world, unsigned int ownerId) {
	character *owner = getActorViaId(ownerId);
	SpellComponent *spellComponent = &world->spell[ownerId];
}

void fireballExit(World *world, unsigned int ownerId) {
	character *owner = getActorViaId(ownerId);
	SpellComponent *spellComponent = &world->spell[ownerId];

	int spellDelay = 2;
	int direction = directionTo(owner->x, owner->y, spellComponent->targetX, spellComponent->targetY);

	createBullet(ownerId, owner->x, owner->y, '*', direction, 3.1, TCOD_color_RGB(200, 40, 40), TCOD_color_RGB(20, 20, 20));

	setStance(owner, IS_RECOVERING);
	setFutureStanceToRemove(owner, IS_RECOVERING);
	setDelay(owner, spellDelay);
}

void fireballHitSolid(World *world, unsigned int entityId) {
	printf("Fireball hit solid\n");
}

void fireballHitActor(World *world, unsigned int entityId) {
	int x1, y1;
	float fx, fy, distance, distanceMod;
	RectComponent *rectComponent = &world->rect[entityId];

	character *target = getActorViaId((unsigned int) rectComponent->collidingWithEntityId);
	target->hp -= 100;

	fx = target->x;
	fy = target->y;
	float size = getRandomInt(2, 3);

	for (y1 = -size; y1 < size; y1 ++) {
		for (x1 = -size; x1 < size; x1 ++) {
			distanceMod = 1 - (distanceFloat(fx, fy, fx + x1, fy + y1) / (float)(size + 1));
			if (distanceMod <= .25 || getRandomFloat(0, 1) > distanceMod) {
				continue;
			}

			bloodSplatter(fx + x1, fy + y1, distanceMod);
		}
	}
}