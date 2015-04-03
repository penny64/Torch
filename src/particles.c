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
