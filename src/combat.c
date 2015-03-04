#include <stdio.h>

#include "libtcod.h"
#include "framework/actors.h"
#include "framework/numbers.h"
#include "lights.h"
#include "player.h"
#include "level.h"
#include "ui.h"


/*int getRandomInt(int min, int max) {
	TCOD_random_t random = TCOD_random_get_instance();
	
	return TCOD_random_get_int(random, min, max);
}*/

int getRandomIntWithMean(int min, int max, int mean) {
	TCOD_random_t random = TCOD_random_get_instance();
	
	return TCOD_random_get_int_mean(random, min, max, mean);
}

float getTargetOpenness(character *target) {
	float openness;
	
	if (target->stanceFlags & IS_STANDING) {
		if (target->stanceFlags & IS_HOLDING_LODGED_WEAPON) {
			openness = .65; // Take armor into account
		} else {
			openness = .5; // Take armor into account
		}
	} else if (target->stanceFlags & IS_SWINGING) {
		openness = .75;
	} else if (target->stanceFlags & IS_STABBING) {
		openness = .65;
	} else if (target->stanceFlags & IS_KICKING) {
		if (target->stanceFlags & IS_HOLDING_LODGED_WEAPON) {
			openness = .7;
		} else {
			openness = .55;
		}
	} else if (target->stanceFlags & IS_CRAWLING) {
		openness = .9;
	} else {
		printf("Could not find openness for current stance.\n");
		
		openness = .5;
	}
	
	return openness;
}

void considerKnockback(character *attacker, character *target, int attackDamage, float percentageAttackDamage) {
	if (target->stanceFlags & IS_CRAWLING) {
		return;
	}
	
	character *player = getPlayer();
	float targetHealth = target->hp;
	float percentageOfTargetHealthAfterDamage = 1 - ((targetHealth - attackDamage) / targetHealth);
	
	if (percentageOfTargetHealthAfterDamage >= .4) {
		if (!(target->stanceFlags & IS_STUNNED)) {
			//target->stanceFlags |= IS_STUNNED;
			setStance(target, IS_STUNNED);
			setFutureStanceToRemove(target, IS_STUNNED);
			setDelay(target, 5);
			
			if (attacker == player) {
				showMessage("%cTarget stunned!%c", 4);
			}
		} else {
			if (target->stanceFlags & IS_STANDING) {
				//target->stanceFlags ^= IS_STANDING;
				//target->stanceFlags |= IS_CRAWLING;
				unsetStance(target, IS_STANDING);
				setStance(target, IS_CRAWLING);
				setDelay(target, 8);
				
				if (attacker == player) {
					showMessage("%cTarget knocked down!%c", 4);
				}
			}
		}
	}
}

int performDamage(character *attacker, character *target, float attackDamage, float percentageAttackDamage) {
	character *player = getPlayer();
	int damageReadout;
	
	if (attacker == player) {
		if (percentageAttackDamage >= .75) {
			showMessage("%cSolid hit.%c", 4);
		}/* else if (percentageAttackDamage >= .5) {
			showMessage("%cOK hit.%c", 4);
		} else {
			showMessage("%cClumsy!%c", 4);
		}*/
	}
	
	damageReadout = attackDamage + .5;
	
	printf("Hitting for: %i\n", damageReadout);
	
	target->hp -= damageReadout;
	
	if (target->hp <= 0) {
		killActor(target);
		
		if (attacker == player) {
			showMessage("%cSplat!%c", 7);
		}

		return 1;
	}

	return 0;
}

int punch(character *attacker, character *target) {
	int attackerStrength = 5;
	int attackerLevel = 3;
	int attackerLuck = 2;
	
	attackerLuck = getRandomIntWithMean(0, attackerLuck, clip(attackerLuck - 1, 0, attackerLuck));
	
	float targetOpenness = getTargetOpenness(target);
	int lowerDamageValue = attackerStrength;
	int upperDamageValue = (attackerStrength + 2) + attackerLevel + attackerLuck;
	int damageMean = upperDamageValue * (1.3 * targetOpenness);;
	float attackDamage, percentageAttackDamage;
	
	setStance(attacker, IS_PUNCHING);
	setFutureStanceToRemove(attacker, IS_PUNCHING);
	setDelay(attacker, 2);
	
	attackDamage = getRandomIntWithMean(lowerDamageValue, upperDamageValue, damageMean);
	percentageAttackDamage = attackDamage / upperDamageValue;
	
	if ((attacker->traitFlags & TORCH_ATTACK_PENALTY && attacker->itemLight)) {
		attackDamage /= 2;

		attacker->itemLight->fuel -= 25;
	}
	
	printf("lower=%i, upper=%i, perc=%f, actual=%f\n", lowerDamageValue, upperDamageValue, percentageAttackDamage, attackDamage);
	
	considerKnockback(attacker, target, attackDamage, percentageAttackDamage);
	
	if (target->stanceFlags & IS_STUCK_WITH_LODGED_WEAPON && getRandomInt(0, getItemLodgedInActor(target)->statDamage)) {
		attackDamage *= 1.2;
		
		showMessage("%cYou punch the lodged sword!%c", 10);
	}
	
	return performDamage(attacker, target, attackDamage, percentageAttackDamage);
}

int slash(character *attacker, character *target, item *weapon) {
	character *player = getPlayer();
	int attackerStrength = 5;
	int attackerLevel = 3;
	int attackerLuck = 2;
	int weaponDamage = getRandomIntWithMean(0, weapon->statDamage, weapon->statDamage + 2);
	
	attackerLuck = getRandomIntWithMean(0, attackerLuck, clip(attackerLuck - 1, 0, attackerLuck));
	
	float targetOpenness = getTargetOpenness(target);
	int lowerDamageValue = attackerStrength;
	int upperDamageValue = (attackerStrength + 2) + attackerLevel + attackerLuck;
	int damageMean = (upperDamageValue * (1.3 * targetOpenness)) + weaponDamage;
	float attackDamage, percentageAttackDamage;
	
	setStance(attacker, IS_SWINGING);
	setFutureStanceToRemove(attacker, IS_SWINGING);
	setDelay(attacker, weapon->statSpeed);
	
	attackDamage = getRandomIntWithMean(lowerDamageValue, upperDamageValue, damageMean);
	percentageAttackDamage = attackDamage / upperDamageValue;
	
	if ((attacker->traitFlags & TORCH_ATTACK_PENALTY && attacker->itemLight)) {
		attackDamage /= 2;

		attacker->itemLight->fuel -= 25;
	}
	
	printf("lower=%i, upper=%i, perc=%f, actual=%f\n", lowerDamageValue, upperDamageValue, percentageAttackDamage, attackDamage);
	
	if (performDamage(attacker, target, attackDamage, percentageAttackDamage)) {
		return 1;
	}
	
	if (getRandomInt(0, weaponDamage)) {
		weapon->itemFlags |= IS_LODGED;
		weapon->lodgedInActor = target;
		attacker->stanceFlags |= IS_HOLDING_LODGED_WEAPON;
		target->stanceFlags |= IS_STUCK_WITH_LODGED_WEAPON;
		
		if (attacker == player) {
			showMessage("%cThe weapon becomes lodged!%c", 10);
		}
	}
	
	return 0;
}

int attack(character *attacker, character *target) {
	item *attackerWeapon = actorGetItemWithFlag(attacker, IS_WEAPON);
	
	if (!attackerWeapon) {
		return punch(attacker, target);
	}
	
	if (attackerWeapon->itemFlags & IS_LODGED) {
		if (target->stanceFlags & IS_STUCK_WITH_LODGED_WEAPON) {
			setFutureStanceToRemove(attacker, IS_HOLDING_LODGED_WEAPON);
			setFutureStanceToRemove(target, IS_STUCK_WITH_LODGED_WEAPON);
			setDelay(attacker, attackerWeapon->statSpeed);
			setDelay(target, attackerWeapon->statSpeed);
		} else {
			printf("*FATAL* Weapon is lodged in target that we aren't attacking\n");
		}
		
		return 0;
	}
	
	if (attackerWeapon->itemFlags & IS_SWORD) {
		return slash(attacker, target, attackerWeapon);
	}
	
	printf("Something terrible has happened.\n");
	
	return 0;
}
