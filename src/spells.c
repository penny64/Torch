#include <stdio.h>

#include "framework/actors.h"
#include "framework/input.h"
#include "spells.h"
#include "systems.h"
#include "entities.h"
#include "components.h"

void spellHandler(World*, unsigned int);
void spellInputHandler(World*, unsigned int);
void fireball(character*, character*);

Spell SPELL_FIREBALL = {&fireball, SPELL_IS_FLAME};


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
}


void castSpell(World *world, unsigned int entityId) {
	SpellComponent *spellComponent = &world->spell[entityId];
	character *owner = getActorViaId(entityId);
	character *target = getActorViaId(entityId);

	spellComponent->castSpell[spellComponent->activeSpell](owner, target);
}

//Spells

void fireball(character *owner, character *target) {
	printf("Boom!\n");
}