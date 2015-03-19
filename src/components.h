#define MAX_SPELLS 10

#include "framework/actors.h"
//#include "entities.h"


#ifndef COMPON_H
#define COMPON_H

typedef enum {
	COMPONENT_NONE = 0,
	COMPONENT_SPELL = 1 << 0,
	COMPONENT_INPUT = 1 << 1,
	COMPONENT_TICK = 1 << 2,
} Component;

typedef enum {
	SPELL_NONE = 0,
	SPELL_IS_FLAME = 1 << 0,
	SPELL_IS_LIGHTNING = 1 << 1,
} SpellTraits;

typedef struct {
	void (*castSpell[MAX_SPELLS])(struct World*, unsigned int, unsigned int);
	unsigned int spellTraits[MAX_SPELLS];
	int spellCount, activeSpell, castDelay[MAX_SPELLS];
} SpellComponent;

#endif
