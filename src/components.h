#define MAX_SPELLS 10

#include "framework/actors.h"


#ifndef COMPON_H
#define COMPON_H

struct World;

typedef enum {
	COMPONENT_NONE = 0,
	COMPONENT_OCCUPIED = 1 << 0,
	COMPONENT_SPELL = 1 << 1,
	COMPONENT_RECT = 1 << 2,
	COMPONENT_SPELL_BULLET = 1 << 3,
	COMPONENT_LIGHT = 1 << 4,
	COMPONENT_COLLIDABLE_SOLID = 1 << 5,
	COMPONENT_COLLIDABLE_ACTOR = 1 << 6,
	COMPONENT_PLAYER = 1 << 7,
} Component;

typedef enum {
	EVENT_NONE = 0,
	EVENT_TICK = 1 << 0,
	EVENT_INPUT = 1 << 1,
	EVENT_DELETED = 1 << 2,
	EVENT_MOVED = 1 << 3,
	EVENT_ADD_STANCE = 1 << 4,
	EVENT_REMOVE_STANCE = 1 << 5,
	EVENT_COLLISION_SOLID = 1 << 6,
	EVENT_COLLISION_ACTOR = 1 << 7,
	EVENT_DRAW = 1 << 8,
} Event;

typedef enum {
	SPELL_NONE = 0,
	SPELL_IS_FLAME = 1 << 0,
	SPELL_IS_LIGHTNING = 1 << 1,
	SPELL_IS_AIMABLE = 1 << 2,
	SPELL_CAST_ON_ENTER = 1 << 3, //Not used (meta only) - use castSpell callback
	SPELL_CAST_ON_EXIT = 1 << 4, //Not used (meta only) - use exitSpell callback
} SpellTraits;

typedef struct {
	char *name[MAX_SPELLS];
	void (*castSpell[MAX_SPELLS])(struct World*, unsigned int);
	void (*tickSpell[MAX_SPELLS])(struct World*, unsigned int);
	void (*exitSpell[MAX_SPELLS])(struct World*, unsigned int);
	unsigned int spellTraits[MAX_SPELLS];
	int spellCount, activeSpell, castDelay[MAX_SPELLS];
	int targetX, targetY, targetEntityId;
} SpellComponent;

typedef struct {
	char chr;
	int x, y, ownerId, collidingWithEntityId;
	double exactX, exactY, velocity[2];
	TCOD_color_t foreColor, backColor;
} RectComponent;

typedef struct {
	unsigned int lightId;
} LightComponent;

#endif
