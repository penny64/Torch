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

	createSystemHandler(world, EVENT_TICK, COMPONENT_RECT, &rectTickHandler);
	createSystemHandler(world, EVENT_DRAW, COMPONENT_RECT, &rectDrawHandler);
}

void registerRectSystem(World *world, unsigned int entityId, int x, int y, int chr, int direction, float speed, TCOD_console_t console, TCOD_color_t foreColor, TCOD_color_t backColor) {
	world->mask[entityId] |= COMPONENT_RECT;

	RectComponent *rectComponent = &world->rect[entityId];

	rectComponent->x = x;
	rectComponent->y = y;
	rectComponent->chr = chr;
	rectComponent->exactX = x;
	rectComponent->exactY = y;
	rectComponent->console = console;
	rectComponent->foreColor = foreColor;
	rectComponent->backColor = backColor;
	rectComponent->ownerId = -1;
	rectComponent->collidingWithEntityId = -1;
	rectComponent->backgroundFlag = TCOD_BKGND_ALPHA(.1);


	velocity(rectComponent->velocity, direction, speed);

	printf("Added entity ID=%i to system `rects`.\n", entityId);
}

void registerRectCollisionSystem(World *world, unsigned int entityId) {
	world->mask[entityId] |= COMPONENT_COLLIDABLE_SOLID | COMPONENT_COLLIDABLE_ACTOR;

	printf("Added entity ID=%i to system `rects-collisions`.\n", entityId);
}

void rectTickHandler(World *world, unsigned int entityId) {
	int x, y, pX, pY, collisionType = 0;
	float vx, vy, tvx, tvy, ratio;

	RectComponent *rectComponent = &world->rect[entityId];

	vx =  rectComponent->velocity[0];
	vy =  rectComponent->velocity[1];

	while (vx || vy) {
		if ((world->mask[entityId] & COMPONENT_COLLIDABLE_SOLID) && isCollidingWithSolid(rectComponent->x, rectComponent->y)) {
			collisionType = EVENT_COLLISION_SOLID;

			tickSystemsWithMaskForEntity(world, entityId, EVENT_COLLISION_SOLID);

		} else if (world->mask[entityId] & COMPONENT_COLLIDABLE_ACTOR) {
			rectComponent->collidingWithEntityId = isCollidingWithActor(rectComponent->x, rectComponent->y, rectComponent->ownerId);

			if (rectComponent->collidingWithEntityId != -1){
				collisionType = EVENT_COLLISION_ACTOR;

				tickSystemsWithMaskForEntity(world, entityId, EVENT_COLLISION_ACTOR);
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
		pX = rectComponent->x;
		pY = rectComponent->y;
		rectComponent->x = x;
		rectComponent->y = y;

		if (pX != x || pY != y) {
			tickSystemsWithMaskForEntity(world, entityId, EVENT_MOVED);
		}

		vx -= tvx;
		vy -= tvy;
	}

	//rectComponent->x = (int)(rectComponent->exactX + .5);
	//rectComponent->y = (int)(rectComponent->exactY + .5);
}

void rectDrawHandler(World *world, unsigned int entityId) {
	RectComponent *rectComponent = &world->rect[entityId];

	drawCharEx(rectComponent->console, rectComponent->x, rectComponent->y, rectComponent->chr, rectComponent->foreColor, rectComponent->backColor, rectComponent->backgroundFlag);
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