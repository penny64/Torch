#include "libtcod.h"


static const int FPS = 60;
static const int WINDOW_WIDTH = 80;
static const int WINDOW_HEIGHT = 50;
static char *WINDOW_TITLE = "Torch and Darkness - flagsdev 2015";
static TCOD_console_t LEVEL_CONSOLE;
static TCOD_console_t ACTOR_CONSOLE;

void createDisplay();
void displayLogic();
