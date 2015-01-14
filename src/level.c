#include "framework/display.h"
#include "framework/draw.h"
#include "level.h"
#include "libtcod.h"


TCOD_console_t LEVEL_CONSOLE;
TCOD_console_t SHADOW_CONSOLE;
TCOD_console_t FOG_CONSOLE;
TCOD_map_t LEVEL_MAP;
TCOD_noise_t FOG_NOISE;

void levelSetup() {
	LEVEL_CONSOLE = TCOD_console_new(WINDOW_WIDTH, WINDOW_HEIGHT);
	SHADOW_CONSOLE = TCOD_console_new(WINDOW_WIDTH, WINDOW_HEIGHT);
	FOG_CONSOLE = TCOD_console_new(WINDOW_WIDTH, WINDOW_HEIGHT);
	LEVEL_MAP = TCOD_map_new(WINDOW_WIDTH, WINDOW_HEIGHT);
	FOG_NOISE = TCOD_noise_new(2, TCOD_NOISE_DEFAULT_HURST, TCOD_NOISE_DEFAULT_LACUNARITY, NULL);

	TCOD_console_set_default_background(LEVEL_CONSOLE, TCOD_color_RGB(40, 30, 30));
	TCOD_console_set_default_background(SHADOW_CONSOLE, TCOD_color_RGB(255, 255, 255));
	TCOD_console_set_key_color(SHADOW_CONSOLE, TCOD_color_RGB(255, 255, 255));

	TCOD_console_clear(LEVEL_CONSOLE);
	TCOD_console_clear(SHADOW_CONSOLE);
	
	TCOD_noise_set_type(FOG_NOISE, TCOD_NOISE_PERLIN);
}

TCOD_console_t getLevelConsole() {
	return LEVEL_CONSOLE;
}

TCOD_console_t getShadowConsole() {
	return SHADOW_CONSOLE;
}

TCOD_console_t getFogConsole() {
	return FOG_CONSOLE;
}

TCOD_noise_t getFogNoise() {
	return FOG_NOISE;
}

TCOD_map_t getLevelMap() {
	return LEVEL_MAP;
}

TCOD_map_t copyLevelMap() {
	TCOD_map_t newMap = TCOD_map_new(WINDOW_WIDTH, WINDOW_HEIGHT);
	TCOD_map_copy(LEVEL_MAP, newMap);
	
	return newMap;
}

int isPositionWalkable(int x, int y) {
	return TCOD_map_is_walkable(LEVEL_MAP, x, y);
}

void generateLevel() {
	int x, y;
	float fogValue;
	float p[2];
	TCOD_noise_t fog = getFogNoise();
	
	for (y = 0; y < WINDOW_HEIGHT; y++) {
		for (x = 0; x < WINDOW_WIDTH; x++) {
			if (x > 10 && x < 20 && y > 10 && y < 20) {
				drawCharBackEx(FOG_CONSOLE, x, y, TCOD_color_RGB(70, 120, 100), TCOD_BKGND_SET);

				TCOD_map_set_properties(LEVEL_MAP, x, y, 0, 0);
			} else {
				p[0] = (float) x / WINDOW_WIDTH;
				p[1] = (float) y / WINDOW_HEIGHT;

				fogValue = TCOD_noise_get_fbm_ex(fog, p, 32.0f, TCOD_NOISE_PERLIN) + .2f;

				if (fogValue < 0) {
					fogValue = 0;
				}

				if (fogValue > .6) {
					fogValue = .6;
				}

				drawCharBackEx(FOG_CONSOLE, x, y, TCOD_color_RGB(235, 200, 200), TCOD_BKGND_ALPHA(fogValue));
				TCOD_map_set_properties(LEVEL_MAP, x, y, 1, 1);
			}
		}
	}
}
