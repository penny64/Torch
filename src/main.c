#include <stdio.h>

#include "framework/logging.h"
#include "framework/actors.h"
#include "framework/display.h"
#include "framework/input.h"
#include "player.h"


void setup() {
	startLogging(LOGGING_DEBUG);
	startInput();
	createDisplay();
	logString(LOGGING_DEBUG, WINDOW_TITLE);
}

int main() {
	setup();
	
	createActor();
	createPlayer();
	
	while (1) {
		inputLogic();
		playerInputLogic();
		actorLogic();
		
		if (isTCODCharPressed(TCODK_ESCAPE)) {
			break;
		}
	
		drawActors();
		
		//printf("%i\n", TCOD_sys_get_fps());
		
		displayLogic();
	}
	
	return 0;
}
