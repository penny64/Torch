#include "input.h"
#include "libtcod.h"


void inputLogic() {
	KEY = TCOD_console_check_for_keypress(TCOD_KEY_PRESSED);
}

int isCharPressed(char char_char) {
	if (char_char == KEY.c) {
		return 1;
	}
	
	return 0;
}

int isTCODCharPressed(int tcod_char) {
	if (tcod_char == KEY.vk) {
		return 1;
	}
	
	return 0;
}

int getInputVk() {
	return KEY.vk;
}
