#include <stdio.h>

#include "framework/logging.h"
#include "framework/actors.h"
#include "framework/display.h"
#include "framework/input.h"
#include "framework/numbers.h"
#include "systems.h"
#include "entities.h"
#include "spells.h"
#include "rects.h"
#include "graphics.h"
#include "particles.h"
#include "lights.h"
#include "player.h"
#include "level.h"
#include "items.h"
#include "intro.h"
#include "ui.h"


int ATTRACT_TIME = 0, ATTRACT_TIME_MAX = 8;
int GAME_DELAY = 0;
int GAME_DELAY_MAX = 8;


void setup() {
	//Setup systems
	startLogging(LOGGING_DEBUG);
	logString(LOGGING_DEBUG, WINDOW_TITLE);

	startNumbers();
	startInput();
	createDisplay();
	setupUi();
	actorSetup();
	itemSetup();
	//createPlayer();
	levelSetup();
	startEntities();
	startSystems();
	startSpells();
	startRects();
	startParticles();

	//Attract screen
	generateLevel();
}

int main() {
	character *player = NULL;
	setup();

	//showIntro();
	
	while (!TCOD_console_is_window_closed()) {
		inputLogic();
		tickSystemsWithMask(getWorld(), EVENT_INPUT);
		playerInputLogic();

		if (isTCODCharPressed(TCODK_ESCAPE)) {
			break;
		}
		
		if (!getPlayer() && isTCODCharPressed(TCODK_SPACE)) {
			setLevel(1);
			createPlayer();
			generateLevel();
			
			continue;
		}
		
		player = getPlayer();

		while (!player || (getPlayerMoveCount() && !GAME_DELAY)) {
			if ((!player && !ATTRACT_TIME) || (player && getPlayerMoveCount())) {
				tickSystemsWithMask(getWorld(), EVENT_TICK);
				actorLogic();
				itemLogic();
				lightLogic();
			}
			
			if (!player) {
				break;
			} else {
				if (player->nextStanceFlagsToAdd || player->nextStanceFlagsToRemove) {
					GAME_DELAY = GAME_DELAY_MAX;
				}
			}
		}
		
		if (GAME_DELAY) {
			GAME_DELAY --;
		}
		
		if (!player) {
			if (!ATTRACT_TIME) {
				ATTRACT_TIME = ATTRACT_TIME_MAX;
			} else {
				ATTRACT_TIME --;
			}
		}

		playerLogic();
		uiLogic();

        if (player && levelLogic()) {
            continue;
        }

		graphicsLogic();
		composeScene();
		displayLogic();
		actorCleanup();
	}
	
	actorsShutdown();
	lightsShutdown();
	itemsShutdown();
	levelShutdown();
	
	return 0;
}
