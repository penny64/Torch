#define MAX_SPELLS 10

#include "framework/actors.h"


#ifndef COMPON_H
#define COMPON_H

struct World;

typedef enum {
	COMPONENT_NONE = 0,
	COMPONENT_OCCUPIED = 1 << 0,
	COMPONENT_SPELL = 1 << 1,
	COMPONENT_INPUT = 1 << 2,
	COMPONENT_TICK = 1 << 3,
} Component;

typedef enum {
	SPELL_NONE = 0,
	SPELL_IS_FLAME = 1 << 0,
	SPELL_IS_LIGHTNING = 1 << 1,
} SpellTraits;

typedef struct {
	char *name[MAX_SPELLS];
	void (*castSpell[MAX_SPELLS])(struct World*, unsigned int, unsigned int);
	unsigned int spellTraits[MAX_SPELLS];
	int spellCount, activeSpell, castDelay[MAX_SPELLS];
} SpellComponent;

#endif
