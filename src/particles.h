#include "libtcod.h"

enum {
	EFFECT_NONE = 0,
	EFFECT_FLICKER = 1 << 0,
	EFFECT_FADE = 1 << 1,
} particleEffectFlags;

TCOD_console_t getEffectsConsole(void);
void startParticles(void);
void createParticle(int, int, int, int, float, float, float, unsigned int, TCOD_color_t, TCOD_color_t);
void createBullet(unsigned int, int, int, int, int, float, TCOD_color_t, TCOD_color_t);