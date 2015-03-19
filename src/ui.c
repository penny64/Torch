#include <stdio.h>
#include <string.h>

#include "framework/actors.h"
#include "framework/display.h"
#include "framework/draw.h"
#include "framework/numbers.h"
#include "libtcod.h"
#include "lights.h"
#include "player.h"
#include "ui.h"


TCOD_console_t UI_CONSOLE;
const char *DISPLAY_TEXT;
float DISPLAY_TEXT_TIME, DISPLAY_TEXT_TIME_MAX;
int DISPLAY_TEXT_FADE = 0, FADE_DELAY = 0;


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
	float colorMod;
	//char timeText[50];

	if (DISPLAY_TEXT != NULL) {
		colorMod = DISPLAY_TEXT_TIME / (float)DISPLAY_TEXT_TIME_MAX;

		if (colorMod < 0) {
			colorMod = 0;
		}

		if (colorMod > 1) {
			colorMod = 1;
		}

		TCOD_console_set_color_control(TCOD_COLCTRL_1, TCOD_color_RGB(255 * colorMod, 250 * colorMod, 220 * colorMod), TCOD_color_RGB(0, 0, 0));
		
		drawString(UI_CONSOLE, (WINDOW_WIDTH / 2) - (strlen(DISPLAY_TEXT) / 2), WINDOW_HEIGHT - 1, DISPLAY_TEXT);
	}
}

void _drawTorchFuel() {
	int x, barWidth = WINDOW_WIDTH;
	float fuelMod;
	character *player = getPlayer();
	
	if (!player) {
		return;
	}

	if (player->itemLight) {
		fuelMod = player->itemLight->fuel / (float)player->itemLight->fuelMax;

		barWidth = clip(barWidth * fuelMod, 1, WINDOW_WIDTH - 2);
	} else {
		barWidth = 1;
	}
	
	drawChar(UI_CONSOLE, 1, 0, '[', TCOD_color_RGB(200, 200, 200), TCOD_color_RGB(65, 65, 65));
	drawChar(UI_CONSOLE, WINDOW_WIDTH - 2, 0, ']', TCOD_color_RGB(200, 200, 200), TCOD_color_RGB(65, 65, 65));

	for (x = 2; x < barWidth; x++) {
		drawCharBackEx(UI_CONSOLE, x, 0, TCOD_color_RGB(255 - x, (255 * fuelMod), 150 * fuelMod), TCOD_BKGND_SET);
	}
	
	for (x = barWidth; x < WINDOW_WIDTH - 2; x++) {
		drawCharBackEx(UI_CONSOLE, x, 0, TCOD_color_RGB(35, 35, 35), TCOD_BKGND_SET);
	}
}

void _drawStance() {
	int x = 0, y = WINDOW_HEIGHT - 1;
	char *stanceText = NULL;
	TCOD_color_t textColor = TCOD_color_RGB(255, 255, 255);
	character *player = getPlayer();
	
	if (!player) {
		return;
	}
	
	item *weapon = actorGetItemWithFlag(player, IS_WEAPON);

	if (player->stanceFlags & IS_CASTING) {
		stanceText = "%cCasting%c";
	} else if (weapon) {
		if (player->stanceFlags & IS_MOVING) {
			stanceText = "%cRun.Slash%c";
		} else if (player->stanceFlags & IS_STANDING) {
			stanceText = "%cSlash%c";
		}
	} else {
		if (player->stanceFlags & IS_MOVING) {
			stanceText = "%cMoving%c";
		} else if (player->stanceFlags & IS_STANDING) {
			stanceText = "%cStanding%c";
		}
	}
	
	TCOD_console_set_color_control(TCOD_COLCTRL_1, textColor, TCOD_color_RGB(0, 0, 0));
	
	drawString(UI_CONSOLE, x, y, stanceText);
}

void showMessage(const char *text, int timeInTurns) {
	DISPLAY_TEXT = text;
	DISPLAY_TEXT_TIME = 0;
	DISPLAY_TEXT_TIME_MAX = (float)timeInTurns * 2;
	DISPLAY_TEXT_FADE = 0;
	FADE_DELAY = 0;
}

void drawUi() {
	TCOD_console_clear(UI_CONSOLE);
	
	_drawTorchFuel();
	_drawStance();
	_drawMessage();
}

void uiLogic() {
	if (FADE_DELAY < 5) {
		FADE_DELAY ++;

		return;
	}

	FADE_DELAY = 0;

	if (!DISPLAY_TEXT_FADE) {
		DISPLAY_TEXT_TIME += 3;

		if (DISPLAY_TEXT_TIME >= DISPLAY_TEXT_TIME_MAX + (FPS * .25)) {
			DISPLAY_TEXT_FADE = 1;
		}
	} else if (DISPLAY_TEXT_FADE && DISPLAY_TEXT_TIME > 0) {
		DISPLAY_TEXT_TIME -= 3;
	} else if (DISPLAY_TEXT != NULL) {
		DISPLAY_TEXT = NULL;
	}
}
