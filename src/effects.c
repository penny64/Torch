#include "libtcod.h"
#include "effects.h"
#include "level.h"

#include "framework/draw.h"


void bloodSplatter(int x, int y, float intensity) {
	drawCharBackEx(getLevelConsole(), x, y, TCOD_color_RGB(200, 15, 15), TCOD_BKGND_ALPHA(intensity));
}
