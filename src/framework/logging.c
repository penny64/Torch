#include <stdio.h>

#include "logging.h"


void startLogging(int level) {
	LOGGING_LEVEL = level;
}

void logString(int level, char *text) {
	if (LOGGING_LEVEL >= level) {
		printf("%s\n", text);
	}
}
