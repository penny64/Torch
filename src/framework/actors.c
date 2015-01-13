#include <stdlib.h>

#include "logging.h"
#include "actors.h"


void createActor() {
	character *_c, *_p_c;
	
	_c = malloc(sizeof(character));
	
	if (characters == NULL) {
		logString(LOGGING_DEBUG, "First entity");
		
		characters = _c;
	} else {
		logString(LOGGING_DEBUG, "Second entity");
		
		_p_c = characters;
		
		characters->next = _c;
		_c->prev = _p_c;
	}
}
