#include <stdio.h>

#include "framework/actors.h"
#include "lights.h"


int meleeAttack(character *attacker, character *target) {
	int damage = 10;

	if ((attacker->traitFlags & TORCH_ATTACK_PENALTY && attacker->itemLight)) {
		damage /= 2;

		attacker->itemLight->fuel -= 25;
	}

	printf("Hit!\n");

	target->hp -= damage;
	
	if (target->hp <= 0) {
		killActor(target);

		return 1;
	}

	return 0;
}
