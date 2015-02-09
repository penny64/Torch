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
#include "ui.h"


void setup() {
	//Setup systems
	startLogging(LOGGING_DEBUG);
	logString(LOGGING_DEBUG, WINDOW_TITLE);

	startInput();
	createDisplay();
	setupUi();
	actorSetup();
	itemSetup();
	createPlayer();
	levelSetup();

	//Attract screen
	generateLevel();
}

int main() {
	setup();
	
	while (!TCOD_console_is_window_closed()) {
		inputLogic();
		playerInputLogic();

        if (isTCODCharPressed(TCODK_ESCAPE)) {
            break;
        }
		
		//While loop?
		if (getPlayerMoveCount()) {
			actorLogic();
			itemLogic();
			lightLogic();
		}

		//effectsLogic();
		playerLogic();
		uiLogic();

        if (levelLogic()) {
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
