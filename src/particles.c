#include <stdio.h>

#include "libtcod.h"
#include "entities.h"
#include "particles.h"
#include "rects.h"
#include "framework/numbers.h"
#include "systems.h"
#include "components.h"
#include "level.h"

void particleTickHandler(World*, unsigned int);


void startParticles() {
	World *world = getWorld();

	createSystemHandler(world, EVENT_TICK, COMPONENT_PARTICLE, &particleTickHandler);
	//createSystemHandler(world, EVENT_DRAW, COMPONENT_RECT, &rectDrawHandler);
}

void registerParticle(World *world, unsigned int entityId, float alpha, float fadeRate) {
	world->mask[entityId] |= COMPONENT_PARTICLE;

	ParticleComponent *particleComponent = &world->particle[entityId];
	RectComponent *rectComponent = &world->rect[entityId];

	particleComponent->alpha = alpha;
	particleComponent->fadeRate = fadeRate;
	particleComponent->foreColor = rectComponent->foreColor;
	particleComponent->backColor = rectComponent->backColor;
}

void createParticle(int x, int y, int chr, int direction, float speed, float alpha, float fadeRate, TCOD_color_t foreColor, TCOD_color_t backColor) {
	unsigned int entityId = createEntity(getWorld());
	World *world = getWorld();

	registerRectSystem(world, entityId, x, y, chr, direction, speed, foreColor, backColor);
	registerParticle(world, entityId, alpha, fadeRate);
}

void createBullet(unsigned int ownerId, int x, int y, int chr, int direction, float speed, TCOD_color_t foreColor, TCOD_color_t backColor) {
	unsigned int entityId = createEntity(getWorld());
	World *world = getWorld();

	registerRectSystem(world, entityId, x, y, chr, direction, speed, foreColor, backColor);
	registerRectCollisionSystem(world, entityId);

	world->mask[entityId] |= COMPONENT_SPELL_BULLET | COMPONENT_LIGHT;

	RectComponent *rectComponent = &world->rect[entityId];
	rectComponent->ownerId = ownerId;

	LightComponent *lightComponent = &world->light[entityId];

	light *lght = createDynamicLight(x, y, NULL);

	lightComponent->lightId = lght->entityId;
	lght->r_tint = foreColor.r;
	lght->g_tint = foreColor.g;
	lght->b_tint = foreColor.b;
	lght->brightness = .9;
	lght->size = 3;
	lght->fuel = 20;
	lght->fuelMax = 20;
}


void particleTickHandler(World *world, unsigned int entityId) {
	ParticleComponent *particleComponent = &world->particle[entityId];
	RectComponent *rectComponent = &world->rect[entityId];

	particleComponent->alpha *= particleComponent->fadeRate;

	if (particleComponent->alpha < .35) {
		deleteEntity(world, entityId);

		return;
	}

	TCOD_color_t fadeForeColor;// = TCOD_console_get_char_foreground(getLevelConsole(), rectComponent->x, rectComponent->y);
	TCOD_color_t fadeBackColor = TCOD_console_get_char_background(getLevelConsole(), rectComponent->x, rectComponent->y);
	fadeForeColor = fadeBackColor;

	fadeForeColor = TCOD_color_lerp(fadeForeColor, TCOD_console_get_char_background(getShadowConsole(), rectComponent->x, rectComponent->y), clipFloat(particleComponent->alpha, .55, 1));
	fadeBackColor = TCOD_color_lerp(fadeBackColor, TCOD_console_get_char_background(getShadowConsole(), rectComponent->x, rectComponent->y), clipFloat(particleComponent->alpha, .55, 1));
	rectComponent->foreColor = TCOD_color_lerp(particleComponent->foreColor, fadeForeColor, clipFloat(1 - particleComponent->alpha, .55, 1));
	rectComponent->backColor = TCOD_color_lerp(particleComponent->backColor, fadeBackColor, clipFloat(1 - particleComponent->alpha, .55, 1));

	rectComponent->backgroundFlag = TCOD_BKGND_ALPHA(particleComponent->alpha);
}