#include <stdlib.h>
#include <math.h>

#include "numbers.h"


int distance(int x1, int y1, int x2, int y2) {
	return abs(x2 - x1) + abs(y2 - y1);
}

float distanceFloat(float x1, float y1, float x2, float y2) {
	long x_dist = fabs(x2 - x1);
	long y_dist = fabs(y2 - y1);

	if (x_dist > y_dist) {
		return (float) (y_dist + (x_dist - y_dist));
	} else {
		return (float) (x_dist + (y_dist - x_dist));
	}
}

int clip(int i, int min, int max) {
	const int t = i < min ? min : i;
	return t > max ? max : t;
}

float clipFloat(float f, float min, float max) {
  const float t = f < min ? min : f;
  return t > max ? max : t;
}

int directionTo(int x1, int y1, int x2, int y2) {
	double theta = atan2((y1 - y2), -(x1 - x2));

	if (theta < 0) {
		theta += 2 * M_PI;
	}

	return (int)((theta * (180 / M_PI)) + .5);
}

void velocity(float velocityArray[2], int direction, float speed) {
	double rad = (double)direction * (M_PI / 180);

	velocityArray[0] = (float)(cos(rad) * (double)speed);
	velocityArray[1] = (float)(sin(rad) * (double)speed);
}
