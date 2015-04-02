#include "entities.h"
#include "particles.h"
#include "rects.h"
#include "framework/numbers.h"
#include "components.h"


void createParticle(int x, int y, int direction, float speed) {
	unsigned int entityId = createEntity(getWorld());
	World *world = getWorld();

	registerRectSystem(world, entityId, x, y, direction, speed);
}
