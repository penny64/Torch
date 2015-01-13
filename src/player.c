#include "framework/actors.h"
#include "framework/input.h"
#include "libtcod.h"
#include "player.h"

void createPlayer() {
	PLAYER_ACTOR = createActor();
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