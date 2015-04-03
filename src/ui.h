#include "libtcod.h"
#include "framework/display.h"


void setupUi(void);
void drawUi(void);
void uiLogic(void);
void showMessage(int, char*, ...);
void createMenu(char*[WINDOW_HEIGHT], void (*)(int, char*));
void createCursor(int, int, TCOD_map_t, void (*)(int, int));
void uiInput(void);
int isUiOpen(void);


TCOD_console_t getUiConsole(void);
