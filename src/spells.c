#include <stdio.h>

#include "framework/actors.h"
#include "framework/input.h"
#include "spells.h"
#include "systems.h"
#include "entities.h"
#include "components.h"

void spellHandler(World*, unsigned int);
void spellInputHandler(World*, unsigned int);
void fireball(World*, unsigned int, unsigned int);

Spell SPELL_FIREBALL = {&fireball, SPELL_IS_FLAME, DELAY_SHORT};


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
	}
}

void registerSpellSystem(World *world, unsigned int entityId) {
	world->mask[entityId] |= COMPONENT_SPELL | COMPONENT_INPUT;

	printf("Added entity ID=%i to system `spells`.\n", entityId);
}

void addSpell(World *world, unsigned int entityId, Spell spell) {
	SpellComponent *spellComponent = &world->spell[entityId];

	spellComponent->castSpell[spellComponent->spellCount] = spell.castSpell;
	spellComponent->spellTraits[spellComponent->spellCount] = spell.spellMask;
	spellComponent->castDelay[spellComponent->spellCount] = spell.castDelay;
}


void castSpell(World *world, unsigned int entityId) {
	SpellComponent *spellComponent = &world->spell[entityId];
	unsigned int owner = entityId;
	unsigned int target = entityId;

	spellComponent->castSpell[spellComponent->activeSpell](world, owner, target);
}

//Spells

void fireball(World *world, unsigned int ownerId, unsigned int targetId) {
	character *owner = getActorViaId(ownerId);
	SpellComponent *spellComponent = &world->spell[ownerId];

	setStance(owner, IS_CASTING);
	setFutureStanceToRemove(owner, IS_CASTING);
	setDelay(owner, spellComponent->castDelay[spellComponent->spellCount]);
}