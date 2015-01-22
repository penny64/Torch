#include <stdio.h>

#include "framework/actors.h"
#include "framework/display.h"
#include "framework/draw.h"
#include "libtcod.h"
#include "lights.h"
#include "player.h"
#include "ui.h"


TCOD_console_t UI_CONSOLE;
const char *DISPLAY_TEXT;
int DISPLAY_TEXT_TIME, DISPLAY_TEXT_TIME_MAX;


void setupUi() {
	UI_CONSOLE = TCOD_console_new(WINDOW_WIDTH, WINDOW_HEIGHT);

	TCOD_console_set_color_control(TCOD_COLCTRL_1, TCOD_color_RGB(200, 200, 200), TCOD_color_RGB(25, 25, 25));
	TCOD_console_set_default_background(UI_CONSOLE, TCOD_color_RGB(255, 0, 255));
	TCOD_console_set_key_color(UI_CONSOLE, TCOD_color_RGB(255, 0, 255));
	TCOD_console_clear(UI_CONSOLE);
}

TCOD_console_t getUiConsole() {
	return UI_CONSOLE;
}

void _drawMessage() {
	//char timeText[50];
	float colorMod;

	if (DISPLAY_TEXT != NULL) {
		colorMod = DISPLAY_TEXT_TIME / (float)DISPLAY_TEXT_TIME_MAX;

		if (colorMod < .5) {
			colorMod = .5;
		}

		TCOD_console_set_color_control(TCOD_COLCTRL_1, TCOD_color_RGB(255 * colorMod, 250 * colorMod, 220 * colorMod), TCOD_color_RGB(25, 25, 25));
		//sprintf(timeText, "%c%i%c", DISPLAY_TEXT_TIME_MAX);

		//drawString(UI_CONSOLE, 0, WINDOW_HEIGHT - 1, timeText);
		drawString(UI_CONSOLE, 3, WINDOW_HEIGHT - 1, DISPLAY_TEXT);
	}
}

void _drawTorchFuel() {
	int x, barWidth = WINDOW_WIDTH;
	float fuelMod;
	character *player = getPlayer();

	fuelMod = player->itemLight->fuel / (float)player->itemLight->fuelMax;
	barWidth *= fuelMod;

	for (x = 0; x <= barWidth; x++) {
		drawCharBackEx(UI_CONSOLE, x, 0, TCOD_color_RGB(255 - x, (255 * fuelMod), 150 * fuelMod), TCOD_BKGND_SET);
	}
}

void showMessage(const char *text, int timeInTurns) {
	DISPLAY_TEXT = text;
	DISPLAY_TEXT_TIME = timeInTurns;
	DISPLAY_TEXT_TIME_MAX = timeInTurns;
}

void drawUi() {
	TCOD_console_clear(UI_CONSOLE);
	
	_drawTorchFuel();
	_drawMessage();
}

void uiLogic() {
	if (DISPLAY_TEXT_TIME > 0) {
		DISPLAY_TEXT_TIME --;
	} else if (DISPLAY_TEXT != NULL) {
		DISPLAY_TEXT = NULL;
	}
}