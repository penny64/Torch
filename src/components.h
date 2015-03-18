#include "framework/actors.h"


#ifndef COMPON_H
#define COMPON_H

typedef enum {
	COMPONENT_NONE = 0,
	COMPONENT_SPELL = 1 << 0,
} Component;

typedef enum {
	SPELL_NONE = 0,
	SPELL_IS_FLAME = 1 << 0,
	SPELL_IS_LIGHTNING = 1 << 1,
} SpellTraits;

typedef struct {
	void (*castSpell)(character*, character*);
	unsigned int spellTraits;
} Spell;

#endif
