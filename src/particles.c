#include "libtcod.h"
#include "entities.h"
#include "particles.h"
#include "rects.h"
#include "framework/numbers.h"
#include "components.h"


void createParticle(int x, int y, int chr, int direction, float speed, TCOD_color_t foreColor, TCOD_color_t backColor) {
	unsigned int entityId = createEntity(getWorld());
	World *world = getWorld();

	registerRectSystem(world, entityId, x, y, chr, direction, speed, foreColor, backColor);
}

void createBullet(unsigned int ownerId, int x, int y, int chr, int direction, float speed, TCOD_color_t foreColor, TCOD_color_t backColor) {
	unsigned int entityId = createEntity(getWorld());
	World *world = getWorld();

	registerRectSystem(world, entityId, x, y, chr, direction, speed, foreColor, backColor);
	registerRectCollisionSystem(world, entityId);

	world->mask[entityId] |= COMPONENT_SPELL_BULLET | COMPONENT_LIGHT | COMPONENT_DELETED;

	RectComponent *rectComponent = &world->rect[entityId];
	rectComponent->ownerId = ownerId;

	LightComponent *lightComponent = &world->light[entityId];

	light *lght = createDynamicLight(x, y, NULL);

	lightComponent->lightId = lght->entityId;
	lght->r_tint = 255;
	lght->g_tint = 40;
	lght->b_tint = 40;
	lght->brightness = .9;
	lght->size = 3;
	lght->fuel = 20;
	lght->fuelMax = 20;
}
