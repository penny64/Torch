#include <stdlib.h>

typedef struct character {
	int x, y, vx, vy, hp;
	struct character *next, *prev;
} character;

character *createActor(void);
void actorLogic(void);
void drawActors(void);

static character *CHARACTERS = NULL;
static character *PLAYER_ACTOR = NULL;