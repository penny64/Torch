#include <stdio.h>

#include "framework/actors.h"
#include "framework/numbers.h"
#include "ai.h"
#include "entities.h"
#include "systems.h"
#include "components.h"

void _aiLogicHandler(World*, unsigned int);
void _aiManageTargets(World*, unsigned int);
void _aiWander(World*, unsigned int);
void _aiTrack(World*, unsigned int);
void _aiHarass(World*, unsigned int);
void _aiRanged(World*, unsigned int);


void startAi() {
	World *world = getWorld();

	createSystemHandler(world, EVENT_TICK, COMPONENT_AI, &_aiLogicHandler);
}

void _registerAi(World *world, unsigned int entityId) {
	world->mask[entityId] |= COMPONENT_AI;

	AiComponent *aiComponent = &world->ai[entityId];

	aiComponent->hasTarget = 0;
	aiComponent->traits = 0;
	aiComponent->trackPosition[0] = 0;
	aiComponent->trackPosition[1] = 0;
}

void registerAiWander(World *world, unsigned int entityId) {
	_registerAi(world, entityId);

	AiComponent *aiComponent = &world->ai[entityId];

	aiComponent->traits |= AI_WANDER;
}

void registerAiTrack(World *world, unsigned int entityId) {
	_registerAi(world, entityId);

	AiComponent *aiComponent = &world->ai[entityId];

	aiComponent->traits |= AI_WANDER | AI_TRACK;
}


//Logic

void _aiManageTargets(World *world, unsigned int entityId) {
	character *closestTarget = NULL, *targetActor = getActors(), *actor = getActorViaId(entityId);
	int closestTargetDistance, targetDistance;

	AiComponent *aiComponent = &world->ai[entityId];

	if (aiComponent->traits == AI_WANDER) {
		return;
	}

	while (targetActor) {
		if (actor == targetActor) {
			targetActor = targetActor->next;

			continue;
		}

		if (TCOD_map_is_in_fov(actor->fov, targetActor->x, targetActor->y)) {
			targetDistance = distance(actor->x, actor->y, targetActor->x, targetActor->y);

			if (!closestTarget || targetDistance < closestTargetDistance) {
				closestTarget = targetActor;
				closestTargetDistance = targetDistance;
			}
		}

		targetActor = targetActor->next;
	}

	if (closestTarget) {
		aiComponent->hasTarget = 1;
		aiComponent->trackPosition[0] = closestTarget->x;
		aiComponent->trackPosition[1] = closestTarget->y;
	}
}

void _aiLogicHandler(World *world, unsigned int entityId) {
	character *actor = getActorViaId(entityId);

	_aiManageTargets(world, entityId);

	if (actor->delay) {
		return;
	}

	AiComponent *aiComponent = &world->ai[entityId];

	if (aiComponent->traits & AI_WANDER) {
		_aiWander(world, entityId);
	}

	if (aiComponent->traits & AI_TRACK) {
		_aiTrack(world, entityId);
	}
}

void _aiWander(World *world, unsigned int entityId) {
	character *actor = getActorViaId(entityId);

	AiComponent *aiComponent = &world->ai[entityId];

	if (aiComponent->hasTarget) {
		return;
	}

	moveActor(actor, getRandomInt(-1, 1), getRandomInt(-1, 1));
}

void _aiTrack(World *world, unsigned int entityId) {
	character *actor = getActorViaId(entityId);

	AiComponent *aiComponent = &world->ai[entityId];

	if (!aiComponent->hasTarget) {
		return;
	}

	walkActor(actor, aiComponent->trackPosition[0], aiComponent->trackPosition[1]);
	walkActorPath(actor);
}