#include <stdio.h>

#include "framework/logging.h"
#include "framework/actors.h"
#include "framework/display.h"
#include "framework/input.h"


void setup() {
	startLogging(LOGGING_DEBUG);
	createDisplay();
	
}

int main() {
	int i = 0, x = 0, y = 0;
	float float_fade_mod;
	
	setup();
	
	logString(LOGGING_DEBUG, WINDOW_TITLE);
	
	createActor();
	createActor();
	
	while (1) {
		inputLogic();
		
		if (isTCODCharPressed(TCODK_ESCAPE)) {
			break;
		}
	
		for (y = 0; y < WINDOW_HEIGHT; y++) {
			for (x = 0; x < WINDOW_WIDTH; x++) {
				float_fade_mod = (float) (x + y + i) / (float) WINDOW_WIDTH / 2;

				drawChar(NULL, x, y, (int)'#', TCOD_color_RGB((int)(255 * float_fade_mod), y, 255), TCOD_color_RGB((int)(255 * float_fade_mod), y, 255));
			}
		}
		
		i++;
		
		printf("%i\n", TCOD_sys_get_fps());
		
		displayLogic();
	}
	
	return 0;
}
