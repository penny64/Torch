#include "input.h"
#include "libtcod.h"


void startInput() {
	TCOD_console_set_keyboard_repeat(10, 0);
}

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
