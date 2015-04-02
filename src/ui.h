#include "libtcod.h"
#include "framework/display.h"


void setupUi(void);
void drawUi(void);
void uiLogic(void);
void showMessage(int, char*, ...);
void createMenu(char*[WINDOW_HEIGHT], void (*)(int, char*));


TCOD_console_t getUiConsole(void);
