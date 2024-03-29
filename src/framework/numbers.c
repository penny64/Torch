#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#include "libtcod.h"
#include "numbers.h"

TCOD_random_t RANDOM;

void startNumbers() {
	RANDOM = TCOD_random_get_instance();
}


int getRandomInt(int min, int max) {
	return TCOD_random_get_int(RANDOM, min, max);
}

float getRandomFloat(float min, float max) {
	return TCOD_random_get_float(RANDOM, min, max);
}

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
	double theta = atan2((y2 - y1), -(x1 - x2));

	if (theta < 0) {
		theta += 2 * M_PI;
	}

	return (int)((theta * (180 / M_PI)) + .5);
}

void velocity(double velocityArray[2], int direction, float speed) {
	double rad = (double)direction * (M_PI / 180);

	velocityArray[0] = (cos(rad) * (double)speed);
	velocityArray[1] = (sin(rad) * (double)speed);
}

int **create2dIntArray(int width, int height) {
	int **array;
	int i;

	array = malloc(width * sizeof(int *));

	for (i = 0; i < width; i ++) {
		array[i] = malloc(height * sizeof(int));
	}

	return array;
}

float **create2dFloatArray(int width, int height) {
	float **array;
	int i;

	array = malloc(width * sizeof(float *));

	for (i = 0; i < width; i ++) {
		array[i] = malloc(height * sizeof(float));
	}

	return array;
}

void delete2dIntArray(int **array, int height) {
	int i;


	for (i = 0; i < height; i ++) {
		free(array[i]);
	}

	free(array);
}

void delete2dFloatArray(float **array, int height) {
	int i;


	for (i = 0; i < height; i ++) {
		free(array[i]);
	}

	free(array);
}