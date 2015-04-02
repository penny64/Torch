#include <stdio.h>

#include "framework/actors.h"
#include "framework/display.h"
#include "framework/input.h"
#include "spells.h"
#include "systems.h"
#include "entities.h"
#include "components.h"
#include "lights.h"
#include "ui.h"


void createCastingMenu(World*, unsigned int);
void spellHandler(World*, unsigned int);
void spellInputHandler(World*, unsigned int);
void fireball(World*, unsigned int, unsigned int);

Spell SPELL_FIREBALL = {"Fireball", &fireball, SPELL_IS_FLAME, DELAY_SHORT};


void startSpells() {
	World *world = getWorld();

	createSystemHandler(world, COMPONENT_SPELL, &spellHandler);
	createSystemHandler(world, COMPONENT_INPUT, &spellInputHandler);
}

void spellHandler(World *world, unsigned int entityId) {
	SpellComponent *spellComponent = &world->spell[entityId];

}

void spellInputHandler(World *world, unsigned int entityId) {
	SpellComponent *spellComponent = &world->spell[entityId];

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

	spellComponent->spellCount ++;
}


void castSpell(World *world, unsigned int entityId) {
	SpellComponent *spellComponent = &world->spell[entityId];
	unsigned int owner = entityId;
	unsigned int target = entityId;

	spellComponent->castSpell[spellComponent->activeSpell](world, owner, target);
}

void _castingMenuCallback(int menuItemIndex, char *menuItemString) {
	printf("Selected item: %s @ %i\n", menuItemString, menuItemIndex);
}

void createCastingMenu(World *world, unsigned int entityId) {
	character *entity = getActorViaId(entityId);
	SpellComponent *spellComponent = &world->spell[entityId];
	int i;
	char *menuStrings[WINDOW_HEIGHT];

	for (i = 0; i < spellComponent->spellCount; i ++) {
		menuStrings[i] = spellComponent->name[i];
	}

	menuStrings[i] = NULL;

	createMenu(menuStrings, &_castingMenuCallback);
}

//Spells

void fireball(World *world, unsigned int ownerId, unsigned int targetId) {
	character *owner = getActorViaId(ownerId);
	SpellComponent *spellComponent = &world->spell[ownerId];
	int x, y, spellDelay;

	x = owner->x;
	y = owner->y;
	spellDelay = spellComponent->castDelay[spellComponent->spellCount];

	light *lght = createDynamicLight(x, y, owner);
	lght->fuel = spellDelay;

	setStance(owner, IS_CASTING);
	setFutureStanceToRemove(owner, IS_CASTING);
	setDelay(owner, spellDelay);
}