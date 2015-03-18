#include "spells.h"
#include "systems.h"
#include "entities.h"

void spellHandler(World*, unsigned int);

void startSpells() {
	createSystemHandler(getWorld(), COMPONENT_SPELL, &spellHandler);
}

void spellHandler(World *world, unsigned int entityId) {

}