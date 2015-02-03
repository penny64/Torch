#include <stdio.h>

#include "framework/actors.h"
#include "lights.h"


void meleeAttack(character *attacker, character *target) {
	int damage = 10;

	if ((attacker->traitFlags & TORCH_ATTACK_PENALTY && attacker->itemLight)) {
		damage /= 2;

		attacker->itemLight->fuel -= 25;
	}

	target->hp -= damage;
	
	if (target->hp <= 0) {
		killActor(target);
	}
}