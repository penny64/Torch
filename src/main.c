#include <stdio.h>

#include "framework/logging.h"
#include "framework/actors.h"
#include "framework/display.h"
#include "framework/input.h"
#include "graphics.h"
#include "lights.h"
#include "player.h"
#include "level.h"
#include "items.h"
#include "intro.h"
#include "ui.h"


int ATTRACT_TIME = 0, ATTRACT_TIME_MAX = 8;


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

	//Attract screen
	generateLevel();
}

int main() {
	setup();

	showIntro();
	
	while (!TCOD_console_is_window_closed()) {
		inputLogic();
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
		
		//While loop?
		while (!getPlayer() || getPlayerMoveCount()) {
			if ((!getPlayer() && !ATTRACT_TIME) || (getPlayer() && getPlayerMoveCount())) {
				actorLogic();
				itemLogic();
				lightLogic();
			}
			
			if (!getPlayer()) {
				break;
			}
		}
		
		if (getPlayer() == NULL) {
			if (!ATTRACT_TIME) {
				ATTRACT_TIME = ATTRACT_TIME_MAX;
			} else {
				ATTRACT_TIME --;
			}
		}

		//effectsLogic();
		playerLogic();
		uiLogic();

        if (getPlayer() && levelLogic()) {
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
