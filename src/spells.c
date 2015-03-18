#include "framework/actors.h"
#include "spells.h"
#include "systems.h"
#include "entities.h"
#include "components.h"

void spellHandler(World*, unsigned int);
void fireball(character*, character*);

Spell SPELL_FIREBALL = {&fireball, SPELL_IS_FLAME};


void startSpells() {
	createSystemHandler(getWorld(), COMPONENT_SPELL, &spellHandler);
}

void spellHandler(World *world, unsigned int entityId) {
	SpellComponent *spellComponent = &world->spell[entityId];


}

void addSpell(World *world, unsigned int entityId, Spell spell) {
	SpellComponent *spellComponent = &world->spell[entityId];

	spellComponent->castSpell[spellComponent->spellCount] = spell.castSpell;
	spellComponent->spellTraits[spellComponent->spellCount] = spell.spellMask;
}


//Spells

void fireball(character *owner, character *target) {

}