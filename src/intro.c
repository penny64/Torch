#include <stdio.h>

#include "framework/display.h"
#include "framework/numbers.h"
#include "framework/input.h"
#include "libtcod.h"
#include "intro.h"


TCOD_console_t INTRO_CONSOLE;
int FADE_IN_VALUE = 0;
float FADE_IN_TIMER = 0;
int FADE_IN_TIMER_MAX = 6;
int INTRO_MODE_TIME = 9;
int FADE_IN = 1;


void setupIntro() {
	INTRO_CONSOLE = TCOD_console_new(WINDOW_WIDTH, WINDOW_HEIGHT);
}

void showIntro() {
	setupIntro();
	introLoop();
}

void introLoop() {
	while (drawIntro()) {
	}	
}

int drawIntro() {
	inputLogic();

	if (FADE_IN_TIMER < FADE_IN_TIMER_MAX) {
		if (FADE_IN) {
			FADE_IN_TIMER ++;
		} else {
			FADE_IN_TIMER += 1.3f;
		}

	} else {
		FADE_IN_TIMER = 0;

		if (FADE_IN) {
			FADE_IN_VALUE += 35;
		} else {
			FADE_IN_VALUE -= 55;
		}

		if (FADE_IN_VALUE > 350) {
			FADE_IN = 0;
		}

		if (!FADE_IN && FADE_IN_VALUE <= 0) {
			INTRO_MODE_TIME --;
		}
	}

	TCOD_console_print_ex(INTRO_CONSOLE, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, TCOD_BKGND_SET, TCOD_CENTER, "%cflagsdev. LLC%c", TCOD_COLCTRL_2, TCOD_COLCTRL_STOP);
	TCOD_console_set_fade(clip(FADE_IN_VALUE, 0, 255), TCOD_color_RGB(0, 0, 0));
	TCOD_console_blit(INTRO_CONSOLE, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, 0, 0, 1, 1);
	TCOD_console_flush();

	if (!INTRO_MODE_TIME || isTCODCharPressed(TCODK_ESCAPE) || isTCODCharPressed(TCODK_SPACE)) {
		TCOD_console_set_fade(255, TCOD_color_RGB(0, 0, 0));

		return 0;
	}

	return INTRO_MODE_TIME;
}