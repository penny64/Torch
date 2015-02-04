#include <stdio.h>

#include "libtcod.h"
#include "framework/display.h"
#include "framework/actors.h"
#include "framework/draw.h"
#include "framework/numbers.h"
#include "lights.h"
#include "graphics.h"
#include "player.h"
#include "level.h"
#include "items.h"
#include "ui.h"

const int FADE_TIME_MAX = 10;
int FADE_TIME = FADE_TIME_MAX;
int FADE_VALUE = 255;


void generateFov() {
	TCOD_map_t map = getLevelMap();
	character *player = getPlayer();
	
	TCOD_map_compute_fov(map, player->x, player->y, 24, 1, FOV_SHADOW);
}

void graphicsLogic() {
	if (!isTransitionInProgress()) {
		return;
	}

	if (FADE_TIME) {
		FADE_TIME --;
	} else {
		FADE_TIME = FADE_TIME_MAX;
		FADE_VALUE = clip(FADE_VALUE - 55, 0, 255);
	}
}

void applyFov() {
	int x, y;
	int visible, visibleToPlayer, isLit;
	float exitWaveDistance = getExitWaveDistance();
	float distMod, distToExitWave, fadeValue;
	float (*effectsMap)[255] = getEffectsMap();
	TCOD_map_t map = getLevelMap();
	TCOD_map_t lightMap = getLightMap();
	TCOD_console_t actorConsole = getActorConsole();
	TCOD_console_t seenConsole = getSeenConsole();
	TCOD_console_t shadowConsole = getShadowConsole();
	character *player = getPlayer();
	character *actor;
	light *lght;

	TCOD_console_clear(shadowConsole);
	
	for (y = 0; y < WINDOW_HEIGHT; y++) {
		for (x = 0; x < WINDOW_WIDTH; x++) {
			visible = 0;
			visibleToPlayer = 1;
			isLit = 0;
			actor = getActors();
			lght = getDynamicLights();
			
			if (TCOD_map_is_walkable(lightMap, x, y)) {
				isLit = 1;
			}
			
			while (lght != NULL && !isLit) {
				if (TCOD_map_is_walkable(lght->lightMap, x, y)) {
					isLit = 1;
					
					break;
				}
				
				lght = lght->next;
			}
			
			while (actor != NULL) {
				if (isLit && TCOD_map_is_in_fov(map, x, y) && TCOD_map_is_in_fov(actor->fov, x, y)) {
					visible = 1;
					
					if (visibleToPlayer && !TCOD_map_is_in_fov(player->fov, x, y)) {
						visibleToPlayer = 0;
					}
				}

				actor = actor->next;
			}
			
			if (!visible) {
				distMod = distanceFloat(player->x, player->y, x, y);
				
				if (distMod < 0) {
					distMod = 0;
				}
				
				fadeValue = 1.f - ((float) distMod / 32.f);
				
				if (fadeValue > .75f) {
					fadeValue = .75f;
				}

				if (isLevelComplete()) {
					distToExitWave = 25.f - clipFloat(25.f * labs(distanceFloat(x, y, player->x, player->y) - exitWaveDistance), 0.f, 25.f);
					distToExitWave = distToExitWave / 25.f;

					effectsMap[x][y] = clipFloat((effectsMap[x][y] + getRandomFloat(-.03, .03) + distToExitWave), .85, 1.25);
				} else {
					effectsMap[x][y] = clipFloat(effectsMap[x][y] + getRandomFloat(-.01, .01), .65, .8);
				}

				fadeValue *= effectsMap[x][y];
				
				setChar(actorConsole, x, y, (int)' ');
				
				if (fadeValue < .03126) {
					drawCharBackEx(shadowConsole, x, y, TCOD_color_RGB(1, 0, 0), TCOD_BKGND_SET);
				} else {
					drawCharBackEx(shadowConsole, x, y, TCOD_color_RGB(35, 35, 35), TCOD_BKGND_ALPHA(fadeValue));
				}
			} else {
				drawCharBack(seenConsole, x, y, TCOD_color_RGB(255, 0, 255));
			}
		}
	}
}

void postProcess() {
	if (!isTransitionInProgress()) {
		return;
	}
	
    TCOD_console_set_fade(FADE_VALUE, TCOD_black);
}

void composeScene() {
	generateFov();
	
	TCOD_console_t levelConsole = getLevelConsole();
	//TCOD_console_t lightConsole = getLightConsole();
	TCOD_console_t dynamicLightConsole = getDynamicLightConsole();
	TCOD_console_t actorConsole = getActorConsole();
	TCOD_console_t itemConsole = getItemConsole();
	TCOD_console_t seenConsole = getSeenConsole();
	TCOD_console_t shadowConsole = getShadowConsole();
	TCOD_console_t fogConsole = getFogConsole();
	TCOD_console_t UiConsole = getUiConsole();
	
	drawDynamicLights();
	applyFov();
	drawUi();
	drawItems();
	
	TCOD_console_blit(levelConsole, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, 0, 0, 1, 1);
	//TCOD_console_blit(lightConsole, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, 0, 0, 0, .9f);
	TCOD_console_blit(dynamicLightConsole, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, 0, 0, 0, .9f);
	TCOD_console_blit(itemConsole, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, 0, 0, 1, 1);
	TCOD_console_blit(actorConsole, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, 0, 0, 1, 0);
	TCOD_console_blit(fogConsole, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, 0, 0, 1, 0.3f);
	TCOD_console_blit(seenConsole, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, 0, 0, 1, 1);
	TCOD_console_blit(shadowConsole, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, 0, 0, 1, 0.55f);
	TCOD_console_blit(UiConsole, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, 0, 0, 1, 1);

	postProcess();
}
