typedef struct character {
	int x, y, hp;
	struct character *next, *prev;
} character;

void createActor(void);

static character *characters = NULL;
