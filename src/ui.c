#include <stdio.h>
#include <string.h>

#include "framework/actors.h"
#include "framework/display.h"
#include "framework/draw.h"
#include "framework/numbers.h"
#include "framework/input.h"
#include "entities.h"
#include "libtcod.h"
#include "lights.h"
#include "graphics.h"
#include "player.h"
#include "ui.h"


TCOD_console_t UI_CONSOLE;
char *DISPLAY_TEXT;
char *MENU_ITEMS[WINDOW_HEIGHT];
float DISPLAY_TEXT_TIME, DISPLAY_TEXT_TIME_MAX, MENU_ITEM_INDEX;
int MENU_ITEM_COUNT, DISPLAY_TEXT_FADE = 0, FADE_DELAY = 0, SHOW_ABILITY_MENU = 1;
void (*MENU_CALLBACK)(int, char*);


void setupUi() {
	UI_CONSOLE = TCOD_console_new(WINDOW_WIDTH, WINDOW_HEIGHT);

	TCOD_console_set_color_control(TCOD_COLCTRL_1, TCOD_color_RGB(200, 200, 200), TCOD_color_RGB(25, 25, 25));
	TCOD_console_set_default_background(UI_CONSOLE, TCOD_color_RGB(0, 0, 0));
	TCOD_console_set_key_color(UI_CONSOLE, TCOD_color_RGB(0, 0, 0));
	TCOD_console_clear(UI_CONSOLE);

	DISPLAY_TEXT = calloc(WINDOW_WIDTH, sizeof(char));
	DISPLAY_TEXT[0] = '\0';
}

TCOD_console_t getUiConsole() {
	return UI_CONSOLE;
}

void _drawMessage() {
	float colorMod;
	TCOD_color_t foreColor, backColor;

	if (DISPLAY_TEXT[0] != '\0') {
		colorMod = DISPLAY_TEXT_TIME / (float)DISPLAY_TEXT_TIME_MAX;

		if (colorMod < 0) {
			colorMod = 0;
		}

		if (colorMod > 1) {
			colorMod = 1;
		}

		foreColor = TCOD_color_RGB(255 * colorMod, 250 * colorMod, 220 * colorMod);
		backColor = TCOD_color_RGB(1, 1, 1);
		
		drawString(UI_CONSOLE, (WINDOW_WIDTH / 2) - (strlen(DISPLAY_TEXT) / 2), WINDOW_HEIGHT - 1, foreColor, backColor, DISPLAY_TEXT);
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
	TCOD_color_t foreColor = TCOD_color_RGB(255, 255, 255);
	TCOD_color_t backColor = TCOD_color_RGB(1, 1, 1);
	character *player = getPlayer();
	
	if (!player) {
		return;
	}

	stanceText = "";
	
	item *weapon = actorGetItemWithFlag(player, IS_WEAPON);

	if (player->stanceFlags & IS_CASTING) {
		stanceText = "Casting";
	} else if (weapon) {
		if (weapon->itemFlags & IS_SWORD) {
			if (player->stanceFlags & IS_MOVING) {
				stanceText = "Run.Slash";
			} else if (player->stanceFlags & IS_STANDING) {
				stanceText = "Slash";
			}
		} else if (weapon->itemFlags & IS_DAGGER) {
			if (player->stanceFlags & IS_STABBING) {
				stanceText = "Stab";
			}
		}
	} else {
		if (player->stanceFlags & IS_MOVING) {
			stanceText = "Moving";
		} else if (player->stanceFlags & IS_STANDING) {
			stanceText = "Standing";
		}
	}
	
	drawString(UI_CONSOLE, x, y, foreColor, backColor, stanceText);
}

void _drawMenu() {
	int i, fadeValue;
	TCOD_color_t cursorColor;

	if (!MENU_CALLBACK) {
		return;
	}

	fadeValue = abs(getAnimateFrame() - 30) * 2;
	cursorColor = TCOD_color_RGB(120 + fadeValue, 120 + fadeValue, 120 + fadeValue);

	TCOD_console_set_default_background(UI_CONSOLE, TCOD_color_RGB(15, 10, 10));
	TCOD_console_set_default_foreground(UI_CONSOLE, TCOD_color_RGB(200, 200, 200));

	for (i = 0; i < MENU_ITEM_COUNT; i ++) {
		if (MENU_ITEM_INDEX == i) {
			drawChar(UI_CONSOLE, 1, 2 + i, '>', cursorColor, TCOD_color_RGB(1, 1, 1));
		}

		TCOD_console_print(UI_CONSOLE, 3, 2 + i, MENU_ITEMS[i]);
	}

	TCOD_console_set_default_background(UI_CONSOLE, TCOD_color_RGB(0, 0, 0));
}

void createMenu(char *menuItems[WINDOW_HEIGHT], void (*callback)(int, char*)) {
	int i;

	memcpy(MENU_ITEMS, menuItems, sizeof(char*) * WINDOW_HEIGHT);

	MENU_CALLBACK = callback;
	MENU_ITEM_INDEX = 0;
	MENU_ITEM_COUNT = 0;

	for (i = 0; i < WINDOW_HEIGHT; i ++) {
		if (MENU_ITEMS[i] == NULL) {
			break;
		}

		MENU_ITEM_COUNT ++;
	}
}

void showMessage(int timeInTurns, char *text, ...) {
	int isSet = 0;
	char *theArg;

	va_list ap;
	theArg = text;

	va_start(ap, text);

	while(theArg != NULL) {
		if (!isSet) {
			strcpy(DISPLAY_TEXT, theArg);

			isSet = 1;
		} else {
			strcat(DISPLAY_TEXT, theArg);
		}

		theArg = va_arg(ap, char*);
	}

	va_end(ap);

	DISPLAY_TEXT_TIME = 0;
	DISPLAY_TEXT_TIME_MAX = (float)timeInTurns * 2;
	DISPLAY_TEXT_FADE = 0;
	FADE_DELAY = 0;
}

void drawUi() {
	TCOD_console_clear(UI_CONSOLE);
	
	_drawTorchFuel();
	_drawStance();
	_drawMenu();
	_drawMessage();
}

void uiInput() {
	if (MENU_ITEM_INDEX) {
		if (isTCODCharPressed(TCODK_UP)) {
			MENU_ITEM_INDEX --;
		}
	}

	if (MENU_ITEM_INDEX < MENU_ITEM_COUNT) {
		if (isTCODCharPressed(TCODK_DOWN)) {
			MENU_ITEM_INDEX ++;
		}
	}
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
	} else if (DISPLAY_TEXT[0] != '\0') {
		DISPLAY_TEXT[0] = '\0';
	}
}
