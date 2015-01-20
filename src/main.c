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


void setup() {
	//Setup systems
	startLogging(LOGGING_DEBUG);
	startInput();
	createDisplay();
	actorSetup();
	itemSetup();
	levelSetup();

	//Generate level just for demonstration purposes
	generateLevel();
	createPlayer();
	createActor();

	logString(LOGGING_DEBUG, WINDOW_TITLE);
}

int main() {
	setup();
	
	while (!TCOD_console_is_window_closed()) {
		inputLogic();
		playerInputLogic();
		
		if (getPlayerMoveCount()) {
			actorLogic();
			lightLogic();
		}
		
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
