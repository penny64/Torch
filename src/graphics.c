#include <stdio.h>
#include <math.h>

#include "libtcod.h"
#include "framework/display.h"
#include "framework/actors.h"
#include "framework/draw.h"
#include "framework/numbers.h"
#include "systems.h"
#include "lights.h"
#include "particles.h"
#include "graphics.h"
#include "player.h"
#include "level.h"
#include "items.h"
#include "ui.h"

const int FADE_TIME_MAX = 10;
int FADE_TIME = 60;
int FADE_VALUE = 255;
int FADE_BACK_IN = 0;
int ANIMATE_TIME = 0;


void generateFov() {
	TCOD_map_t map = getLevelMap();
	character *player = getPlayer();
	
	if (!player) {
		return;
	}
	
	TCOD_map_compute_fov(map, player->x, player->y, 24, 1, FOV_SHADOW);
}

int getAnimateFrame() {
	return ANIMATE_TIME;
}

int isScreenFadedOut() {
	return !FADE_VALUE;
}

int isScreenFadingBackIn() {
	return FADE_BACK_IN;
}

void fadeBackIn() {
	FADE_BACK_IN = 1;
}

void graphicsLogic() {
	character *player = getPlayer();
	
	if (ANIMATE_TIME < 60) {
		ANIMATE_TIME ++;
	} else {
		ANIMATE_TIME = 0;
	}

	if (FADE_BACK_IN) {
		if (FADE_TIME < FADE_TIME_MAX) {
			FADE_TIME ++;
		} else {
			FADE_TIME = 0;
			FADE_VALUE = clip(FADE_VALUE + 55, 0, 255);

			if (FADE_VALUE == 255) {
				FADE_BACK_IN = 0;
				transitionIsComplete();
			}
		}
	} else {
		if (!isTransitionInProgress() || player->itemLight->sizeMod > 0) {
			return;
		}

		if (FADE_TIME) {
			FADE_TIME --;
		} else {
			FADE_TIME = FADE_TIME_MAX;
			FADE_VALUE = clip(FADE_VALUE - 55, 0, 255);
		}
	}
}

void postProcess() {
	if (!isTransitionInProgress()) {
		return;
	}

	TCOD_console_set_fade(FADE_VALUE, TCOD_color_RGB(0, 0, 0));
}

void applyFov() {
	int x, y;
	int visible, visibleToPlayer, isLit;
	float exitWaveDistance = getExitWaveDistance();
	float distMod, distToExitWave, fadeValue;
	float **effectsMap = getEffectsMap();
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
			
			if (lightMap) {
				if (TCOD_map_is_walkable(lightMap, x, y)) {
					isLit = 1;
				}
				
				while (lght != NULL && lght->lightMap && !isLit) {
					if (TCOD_map_is_walkable(lght->lightMap, x, y)) {
						isLit = 1;
						
						break;
					}
					
					lght = lght->next;
				}
			}
			
			while (actor != NULL) {
				if (isLit && TCOD_map_is_in_fov(map, x, y) && TCOD_map_is_in_fov(actor->fov, x, y)) {
					visible = 1;
					
					if (!player || (visibleToPlayer && !TCOD_map_is_in_fov(player->fov, x, y))) {
						visibleToPlayer = 0;
					}
				}

				actor = actor->next;
			}
			
			if (!player) {
				visible = 1;
				visibleToPlayer = 1;
				isLit = 1;
			}
			
			if (!visible) {
				if (player) {
					distMod = distanceFloat(player->x, player->y, x, y);
				} else {
					distMod = 45.0f;
				}
				
				if (distMod < 0) {
					distMod = 0;
				}
				
				fadeValue = 1.f - ((float) distMod / 32.f);
				
				if (fadeValue > .75f) {
					fadeValue = .75f;
				}

				if (isLevelComplete() && player) {
					distToExitWave = 25.f - clipFloat(25.f * fabs(distanceFloat(x, y, player->x, player->y) - exitWaveDistance), 0.f, 25.f);
					distToExitWave = distToExitWave / 25.f;

					effectsMap[x][y] = clipFloat((effectsMap[x][y] + getRandomFloat(-.03, .03) + distToExitWave), .85, 1.05);
				} else {
					effectsMap[x][y] = clipFloat(effectsMap[x][y] + getRandomFloat(-.01, .01), .65, .8);
				}

				fadeValue *= effectsMap[x][y];
				
				setChar(actorConsole, x, y, (int)' ');
				
				if (fadeValue < .039) {
					drawCharBackEx(shadowConsole, x, y, TCOD_color_RGB(1, 0, 0), TCOD_BKGND_SET);
				} else {
					drawCharBackEx(shadowConsole, x, y, TCOD_color_RGB(30, 0, 0), TCOD_BKGND_ALPHA(fadeValue));
				}
			} else {
				drawCharBack(seenConsole, x, y, TCOD_color_RGB(255, 0, 255));
			}
		}
	}
}

void composeScene() {
	generateFov();
	
	TCOD_console_t levelConsole = getLevelConsole();
	TCOD_console_t dynamicLightConsole = getDynamicLightConsole();
	TCOD_console_t actorConsole = getActorConsole();
	TCOD_console_t itemConsole = getItemConsole();
	TCOD_console_t effectsConsole = getEffectsConsole();
	TCOD_console_t seenConsole = getSeenConsole();
	TCOD_console_t shadowConsole = getShadowConsole();
	TCOD_console_t uiConsole = getUiConsole();

	TCOD_console_clear(effectsConsole);

	drawActors();
	drawDynamicLights();
	applyFov();
	drawUi();
	drawItems();
	tickSystemsWithMask(getWorld(), EVENT_DRAW);
	
	TCOD_console_blit(levelConsole, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, 0, 0, 1, 1);
	TCOD_console_blit(dynamicLightConsole, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, 0, 0, 0, .7f);
	TCOD_console_blit(effectsConsole, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, 0, 0, .55f, .55f);
	TCOD_console_blit(itemConsole, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, 0, 0, 1, 0);
	TCOD_console_blit(actorConsole, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, 0, 0, 1, 0);
	TCOD_console_blit(seenConsole, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, 0, 0, 1, 1);
	TCOD_console_blit(shadowConsole, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, 0, 0, 1, .55f);
	TCOD_console_blit(uiConsole, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, 0, 0, 1, 1);

	postProcess();
}
