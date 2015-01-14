#include "player.h"
#include "framework/actors.h"
#include "framework/input.h"
#include "libtcod.h"


static struct character *PLAYER_ACTOR;


void createPlayer() {
	PLAYER_ACTOR = createActor();
}

character *getPlayer() {
	return PLAYER_ACTOR;
}

void playerInputLogic() {
	if (isTCODCharPressed(TCODK_LEFT)) {
		PLAYER_ACTOR->vx = -1;
	} else if (isTCODCharPressed(TCODK_RIGHT)) {
		PLAYER_ACTOR->vx = 1;
	}

	if (isTCODCharPressed(TCODK_UP)) {
		PLAYER_ACTOR->vy = -1;
	} else if (isTCODCharPressed(TCODK_DOWN)) {
		PLAYER_ACTOR->vy = 1;
	}
}
