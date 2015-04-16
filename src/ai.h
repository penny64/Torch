#include "entities.h"

enum {
	AI_NONE = 0,
	AI_WANDER = 1 << 0,
	AI_TRACK = 1 << 1,
	AI_HARASS = 1 << 2,
	AI_RANGED = 1 << 3,
	AI_PATROL = 1 << 4,
} AiFlags_t;


void startAi(void);
void registerAi(World*, unsigned int);
void registerAiWander(World*, unsigned int);
void registerAiPatrol(World*, unsigned int);
void registerAiTrack(World*, unsigned int);
void registerAiHarass(World*, unsigned int);
void registerAiRanged(World*, unsigned int);