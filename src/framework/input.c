#include "input.h"
#include "libtcod.h"
#include "stdio.h"


TCOD_key_t KEY;
TCOD_mouse_t MOUSE;
TCOD_event_t INPUT_EVENT;


void startInput() {
	TCOD_console_set_keyboard_repeat(100, 500);
}

void inputLogic() {
	INPUT_EVENT = TCOD_sys_check_for_event(TCOD_EVENT_KEY, &KEY, &MOUSE);
}

int isCharPressed(char char_char) {
	if (INPUT_EVENT != TCOD_EVENT_KEY_PRESS) {
		return 0;
	}
	
	if (char_char == KEY.c) {
		return 1;
	}
	
	return 0;
}

int isTCODCharPressed(int tcod_char) {
	if (INPUT_EVENT != TCOD_EVENT_KEY_PRESS) {
		return 0;
	}
	
	if (tcod_char == KEY.vk) {
		return 1;
	}
	
	return 0;
}

int getInputVk() {
	return KEY.vk;
}
