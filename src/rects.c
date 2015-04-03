#include <stdio.h>
#include <math.h>

#include "framework/draw.h"
#include "framework/numbers.h"
#include "rects.h"
#include "systems.h"
#include "ui.h"
#include "level.h"
#include "components.h"

void rectTickHandler(World*, unsigned int);
void rectDrawHandler(World*, unsigned int);
int isCollidingWithSolid(int, int);
int isCollidingWithActor(int, int, unsigned int);


void startRects() {
	World *world = getWorld();

	createSystemHandler(world, COMPONENT_TICK, &rectTickHandler);
	createSystemHandler(world, COMPONENT_DRAW, &rectDrawHandler);
}

void registerRectSystem(World *world, unsigned int entityId, int x, int y, int chr, int direction, float speed, TCOD_color_t foreColor, TCOD_color_t backColor) {
	world->mask[entityId] |= COMPONENT_RECT | COMPONENT_TICK | COMPONENT_DRAW;

	RectComponent *rectComponent = &world->rect[entityId];

	rectComponent->x = x;
	rectComponent->y = y;
	rectComponent->chr = chr;
	rectComponent->exactX = x;
	rectComponent->exactY = y;
	rectComponent->foreColor = foreColor;
	rectComponent->backColor = backColor;
	rectComponent->ownerId = -1;

	velocity(rectComponent->velocity, direction, speed);

	printf("Added entity ID=%i to system `rects`.\n", entityId);
}

void registerRectCollisionSystem(World *world, unsigned int entityId) {
	world->mask[entityId] |= COMPONENT_COLLISION_SOLID | COMPONENT_COLLISION_ACTOR;

	printf("Added entity ID=%i to system `rects-collisions`.\n", entityId);
}

void rectTickHandler(World *world, unsigned int entityId) {
	int x, y, collisionType = 0;
	float vx, vy, tvx, tvy, ratio;

	RectComponent *rectComponent = &world->rect[entityId];

	vx =  rectComponent->velocity[0];
	vy =  rectComponent->velocity[1];

	while (vx || vy) {
		if ((world->mask[entityId] & COMPONENT_COLLISION_SOLID) && isCollidingWithSolid(rectComponent->x, rectComponent->y)) {
			collisionType = COMPONENT_COLLISION_SOLID;

			tickSystemsWithMaskForEntity(world, entityId, COMPONENT_COLLISION_SOLID);

		}else if (world->mask[entityId] & COMPONENT_COLLISION_ACTOR) {
			if (isCollidingWithActor(rectComponent->x, rectComponent->y, rectComponent->ownerId) != -1){
				collisionType = COMPONENT_COLLISION_ACTOR;
			}
		}

		if (collisionType) {
			rectComponent->velocity[0] = 0;
			rectComponent->velocity[1] = 0;

			break;
		}

		tvx = vx;
		tvy = vy;

		if (fabs(tvx) > fabs(tvy) && fabs(tvx) > 1) {
			ratio = 1 / fabs(tvx);

			tvy *= ratio;
		} else if (fabs(tvy) > 1) {
			ratio = 1 / fabs(tvy);

			tvx *= ratio;
		} else {
			ratio = 1;
		}

		tvx = clipFloat(tvx, -1, 1);
		tvy = clipFloat(tvy, -1, 1);

		x = (int)((rectComponent->exactX + tvx) + .5);
		y = (int)((rectComponent->exactY + tvy) + .5);

		rectComponent->exactX += tvx;
		rectComponent->exactY += tvy;
		rectComponent->x = x;
		rectComponent->y = y;

		tickSystemsWithMaskForEntity(world, entityId, COMPONENT_MOVED);

		vx -= tvx;
		vy -= tvy;

		//TODO: Check against owner
		/*while (actorPtr != NULL) {
			if (actorPtr == actor || ptr->hp <= 0) {
				ptr = ptr->next;

				continue;
			}

			if (ptr->x == nx && ptr->y == ny) {
				hitActor = 1;

				break;
			}

			ptr = ptr->next;
		}*/
	}

	//rectComponent->exactX += rectComponent->velocity[0];
	//rectComponent->exactY += rectComponent->velocity[1];

	rectComponent->x = (int)(rectComponent->exactX + .5);
	rectComponent->y = (int)(rectComponent->exactY + .5);
}

void rectDrawHandler(World *world, unsigned int entityId) {
	RectComponent *rectComponent = &world->rect[entityId];

	drawChar(getUiConsole(), rectComponent->x, rectComponent->y, rectComponent->chr, rectComponent->foreColor, rectComponent->backColor);
}

int isCollidingWithSolid(int x, int y) {
	return !isPositionWalkable(x, y);
}

int isCollidingWithActor(int x, int y, unsigned int ignoreActorEntityId) {
	character *actorPtr = getActors();

	while (actorPtr) {
		if (actorPtr->entityId != ignoreActorEntityId && actorPtr->x == x && actorPtr->y == y) {
			return actorPtr->entityId;
		}

		actorPtr = actorPtr->next;
	}

	return -1;
}