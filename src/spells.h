#include "entities.h"

#ifndef SPELLS_H
#define SPELLS_H

typedef struct {
	void (*castSpell)(character*, character*);
	unsigned int spellMask;
} Spell;

Spell SPELL_FIREBALL;

#endif


void startSpells(void);
void registerSpellSystem(World*, unsigned int);
void castSpell(World*, unsigned int);
void addSpell(World*, unsigned int, Spell);