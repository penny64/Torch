#include "entities.h"

#ifndef SPELLS_H
#define SPELLS_H

enum {
	DELAY_SHORT = 5,
} delayFlag_t;

typedef struct {
	char *name;
	void (*castSpell)(World*, unsigned int, unsigned int);
	unsigned int spellMask;
	unsigned int parentItem;
	int castDelay;
} Spell;

Spell SPELL_FIREBALL;

#endif


void startSpells(void);
void registerSpellSystem(World*, unsigned int);
void castSpell(World*, unsigned int);
void addSpell(World*, unsigned int, Spell);