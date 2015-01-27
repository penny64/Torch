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
	startInput();
	createDisplay();
	setupUi();
	actorSetup();
	itemSetup();
	createPlayer();
	levelSetup();

	//Attract screen
	generateLevel();
	resetActorForNewLevel(getPlayer());
	createActor();

	logString(LOGGING_DEBUG, WINDOW_TITLE);
}

int main() {
	setup();
	
	while (!TCOD_console_is_window_closed()) {
		inputLogic();
		playerInputLogic();
		
		//While loop?
		if (getPlayerMoveCount()) {
			actorLogic();
			itemLogic();
			lightLogic();
		}

		//effectsLogic();
		uiLogic();
		
		if (isTCODCharPressed(TCODK_ESCAPE)) {
			break;
		}
	
		drawActors();

		composeScene();

		displayLogic();
		
		actorCleanup();
	}
	
	return 0;
}
