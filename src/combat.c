#include <stdio.h>

#include "framework/actors.h"


void meleeAttack(character *attacker, character *target) {
	target->hp -= 50;
	
	printf("Done\n");
	
	if (target->hp <= 0) {
		killActor(target);
	}
}
