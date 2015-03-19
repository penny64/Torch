#include <stdio.h>

#include "framework/logging.h"
#include "framework/actors.h"
#include "framework/display.h"
#include "framework/input.h"
#include "systems.h"
#include "entities.h"
#include "spells.h"
#include "graphics.h"
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

	//Attract screen
	generateLevel();
}

int main() {
	character *player = NULL;
	setup();

	showIntro();
	
	while (!TCOD_console_is_window_closed()) {
		inputLogic();
		tickSystemsWithMask(getWorld(), COMPONENT_INPUT);
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
		
		//While loop?
		while (!player || (getPlayerMoveCount() && !GAME_DELAY)) {
			if ((!player && !ATTRACT_TIME) || (player && getPlayerMoveCount())) {
				tickSystemsWithMask(getWorld(), COMPONENT_TICK);
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

		//effectsLogic();
		playerLogic();
		uiLogic();

        if (player && levelLogic()) {
            continue;
        }

		graphicsLogic();
	
		drawActors();

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
