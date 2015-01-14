#include <stdlib.h>

#include "numbers.h"


int distance(int x1, int y1, int x2, int y2) {
	return abs(x2 - x1) + abs(y2 - y1);
}

float distanceFloat(float x1, float y1, float x2, float y2) {
	return labs(x2 - x1) + labs(y2 - y1);
}