#include <stdio.h>

#include "framework/logging.h"
#include "framework/actors.h"
#include "framework/display.h"
#include "framework/input.h"
#include "graphics.h"
#include "level.h"
#include "player.h"


void setup() {
	//Setup systems
	startLogging(LOGGING_DEBUG);
	startInput();
	createDisplay();
	actorSetup();
	levelSetup();

	//Generate level just for demonstration purposes
	generateLevel();
	createPlayer();

	logString(LOGGING_DEBUG, WINDOW_TITLE);
}

int main() {
	setup();
	
	while (!TCOD_console_is_window_closed()) {
		inputLogic();
		playerInputLogic();
		actorLogic();
		
		if (isTCODCharPressed(TCODK_ESCAPE)) {
			break;
		}
	
		drawActors();

		composeScene();

		displayLogic();
	}
	
	return 0;
}
