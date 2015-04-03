#include <stdio.h>

#include "framework/actors.h"
#include "framework/display.h"
#include "framework/numbers.h"
#include "framework/input.h"
#include "spells.h"
#include "systems.h"
#include "particles.h"
#include "entities.h"
#include "components.h"
#include "lights.h"
#include "ui.h"


void createCastingMenu(World*, unsigned int);
void spellHandler(World*, unsigned int);
void spellInputHandler(World*, unsigned int);
void fireball(World*, unsigned int, unsigned int);
void _spellTargetCursorCallback(int, int);
void createTargetCursor(World*, unsigned int, void (*)(int, int));

Spell SPELL_FIREBALL = {"Fireball", &fireball, SPELL_IS_FLAME | SPELL_IS_AIMABLE, -1, DELAY_SHORT};

int UI_OWNER_ID;
World *UI_WORLD_PTR;


void startSpells() {
	World *world = getWorld();

	createSystemHandler(world, COMPONENT_SPELL, &spellHandler);
	createSystemHandler(world, COMPONENT_INPUT, &spellInputHandler);
}

void spellHandler(World *world, unsigned int entityId) {
	//SpellComponent *spellComponent = &world->spell[entityId];

}

void spellInputHandler(World *world, unsigned int entityId) {
	//SpellComponent *spellComponent = &world->spell[entityId];

	if (isCharPressed('x')) {
		castSpell(world, entityId);
	} else if (isCharPressed('X')) {
		createCastingMenu(world, entityId);
	}
}

void registerSpellSystem(World *world, unsigned int entityId) {
	world->mask[entityId] |= COMPONENT_SPELL | COMPONENT_INPUT;

	printf("Added entity ID=%i to system `spells`.\n", entityId);
}

void addSpell(World *world, unsigned int entityId, Spell spell) {
	SpellComponent *spellComponent = &world->spell[entityId];

	spellComponent->name[spellComponent->spellCount] = spell.name;
	spellComponent->castSpell[spellComponent->spellCount] = spell.castSpell;
	spellComponent->spellTraits[spellComponent->spellCount] = spell.spellMask;
	spellComponent->castDelay[spellComponent->spellCount] = spell.castDelay;
	spellComponent->activeSpell = -1;

	spellComponent->spellCount ++;
}


void castSpell(World *world, unsigned int entityId) {
	SpellComponent *spellComponent = &world->spell[entityId];
	unsigned int owner = entityId;
	unsigned int target = entityId;

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
		spellComponent->castSpell[spellComponent->activeSpell](world, owner, target);
	}
}

void _spellTargetCursorCallback(int x, int y) {
	character *owner = getActorViaId(UI_OWNER_ID);

	int direction = directionTo(owner->x, owner->y, x, y);

	createParticle(owner->x, owner->y, '*', direction, 3.1, TCOD_color_RGB(200, 200, 200), TCOD_color_RGB(20, 20, 20));

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

void fireball(World *world, unsigned int ownerId, unsigned int targetId) {
	character *owner = getActorViaId(ownerId);
	SpellComponent *spellComponent = &world->spell[ownerId];
	int x, y, spellDelay;

	x = owner->x;
	y = owner->y;
	spellDelay = spellComponent->castDelay[spellComponent->activeSpell];

	light *lght = createDynamicLight(x, y, owner);
	lght->r_tint = 200;
	lght->g_tint = 40;
	lght->b_tint = 40;
	lght->fuel = spellDelay;
	lght->fuelMax = spellDelay;

	setStance(owner, IS_CASTING);
	setFutureStanceToRemove(owner, IS_CASTING);
	setDelay(owner, spellDelay);
}