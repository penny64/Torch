#include <stdio.h>

#include "libtcod.h"
#include "framework/actors.h"
#include "framework/numbers.h"
#include "lights.h"
#include "player.h"
#include "level.h"
#include "ui.h"


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
	float forceRequirementToStun = .4;
	
	if (attacker->stanceFlags & IS_MOVING) {
		forceRequirementToStun = .2;
	}
	
	if (percentageOfTargetHealthAfterDamage >= forceRequirementToStun) {
		printf("Good!\n");
		if (!(target->stanceFlags & IS_STUNNED) && target->aiFlags & IS_ALIVE) {
			setStance(target, IS_STUNNED);
			setFutureStanceToRemove(target, IS_STUNNED);
			setDelay(target, 10);
			
			if (attacker == player) {
				showMessage(4, "Target stunned!", NULL);
			}
		} else {
			printf("NO!\n");
			if (target->stanceFlags & IS_STANDING) {
				unsetStance(target, IS_STANDING);
				setStance(target, IS_CRAWLING);
				setDelay(target, 8);
				
				if (attacker == player) {
					showMessage(4, "Target knocked down!", NULL);
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
			showMessage(4, "Solid hit.", NULL);
		}/* else if (percentageAttackDamage >= .5) {
			showMessage("%cOK hit.%c", 4);
		} else {
			showMessage("%cClumsy!%c", 4);
		}*/
	}

	attackDamage = clip(attackDamage - getActorDefense(target), 0, 9999);
	
	damageReadout = attackDamage + .5;
	
	printf("Hitting for: %i\n", damageReadout);
	
	target->hp -= damageReadout;
	
	if (target->hp <= 0) {
		killActor(target);
		
		if (attacker == player) {
			showMessage(7, "Splat!", NULL);
		}

		return 1;
	}

	return 0;
}

int punch(character *attacker, character *target) {
	int attackerStrength = getActorStrength(attacker);
	int attackerLevel = getActorLevel(attacker);
	int attackerLuck = getActorLuck(attacker);
	
	attackerLuck = getRandomIntWithMean(0, attackerLuck, clip(attackerLuck - 1, 0, attackerLuck));
	
	float targetOpenness = getTargetOpenness(target);
	int lowerDamageValue = attackerStrength;
	int upperDamageValue = (attackerStrength + 2) + attackerLevel + attackerLuck;
	int damageMean = upperDamageValue * (1.3 * targetOpenness);
	float attackDamage, percentageAttackDamage, movementDamageBonus = .0f;
	
	if (attacker->stanceFlags & IS_MOVING) {
		movementDamageBonus = (10.f - ((float)getActorSpeed(attacker))) / 10.f;
		movementDamageBonus *= attackerStrength;
		
		printf("Extra movement damage: %f\n", movementDamageBonus);
	}
	
	lowerDamageValue += (int)(movementDamageBonus + .5f);
	upperDamageValue += (int)(movementDamageBonus + .5f);
	damageMean += (int)(movementDamageBonus + .5f);
	
	setStance(attacker, IS_PUNCHING);
	setFutureStanceToRemove(attacker, IS_PUNCHING);
	setDelay(attacker, 2);
	
	attackDamage = getRandomIntWithMean(lowerDamageValue, upperDamageValue, damageMean);
	percentageAttackDamage = attackDamage / upperDamageValue;
	
	if ((attacker->traitFlags & TORCH_ATTACK_PENALTY && !actorGetItemWithFlag(attacker, IS_TORCH_HOLDER) && attacker->itemLight)) {
		attackDamage /= 2;

		attacker->itemLight->fuel -= 25;
	}
	
	printf("lower=%i, upper=%i, perc=%f, actual=%f\n", lowerDamageValue, upperDamageValue, percentageAttackDamage, attackDamage);
	
	considerKnockback(attacker, target, attackDamage, percentageAttackDamage);
	
	if (target->stanceFlags & IS_STUCK_WITH_LODGED_WEAPON && getRandomInt(0, getItemLodgedInActor(target)->statDamage)) {
		attackDamage *= 1.2;
		
		showMessage(10, "You punch the lodged sword!", NULL);
	}
	
	return performDamage(attacker, target, attackDamage, percentageAttackDamage);
}

int slash(character *attacker, character *target, item *weapon) {
	character *player = getPlayer();
	int attackerStrength = getActorStrength(attacker);
	int attackerLevel = getActorLevel(attacker);
	int attackerLuck = getActorLuck(attacker);
	int weaponDamage = getRandomIntWithMean(0, weapon->statDamage, weapon->statDamage + (2 + attackerLevel));
	float recoveryTime = getAttackSpeedOfWeapon(weapon) * getActorStability(attacker);
	
	attackerLuck = getRandomIntWithMean(0, attackerLuck, clip(attackerLuck - 1, 0, attackerLuck));
	
	float targetOpenness = getTargetOpenness(target);
	int lowerDamageValue = attackerStrength;
	int upperDamageValue = (attackerStrength + 2) + attackerLevel + attackerLuck;
	int damageMean = (upperDamageValue * (1.3 * targetOpenness)) + weaponDamage;
	float attackDamage, percentageAttackDamage, movementDamageBonus = .0f;
	
	if (attacker->stanceFlags & IS_MOVING) {
		movementDamageBonus = (10.f - ((float)getActorSpeed(attacker))) / 10.f;
		movementDamageBonus *= attackerStrength;
		
		printf("Extra movement damage: %f\n", movementDamageBonus);
	}
	
	lowerDamageValue += (int)(movementDamageBonus + .5f);
	upperDamageValue += (int)(movementDamageBonus + .5f);
	damageMean += (int)(movementDamageBonus + .5f);
	
	setStance(attacker, IS_SWINGING);
	setFutureStanceToRemove(attacker, IS_SWINGING);
	setDelay(attacker, recoveryTime);
	
	attackDamage = getRandomIntWithMean(lowerDamageValue, upperDamageValue, damageMean);
	percentageAttackDamage = attackDamage / upperDamageValue;

	float attackDamageBeforeDefense = attackDamage;
	
	if ((attacker->traitFlags & TORCH_ATTACK_PENALTY && !actorGetItemWithFlag(attacker, IS_TORCH_HOLDER) && attacker->itemLight)) {
		attackDamage /= 2;

		attacker->itemLight->fuel -= 25;
	}
	
	printf("lower=%i, upper=%i, perc=%f, actual=%f\n", lowerDamageValue, upperDamageValue, percentageAttackDamage, attackDamage);
	
	if (performDamage(attacker, target, attackDamage, percentageAttackDamage)) {
		return 1;
	}
	
	if (attackDamageBeforeDefense <= upperDamageValue * .85 && getRandomFloat(0.f, 1.f) > (attackDamageBeforeDefense / (upperDamageValue * .85)) / (upperDamageValue * .85)) {
		weapon->itemFlags |= IS_LODGED;
		weapon->lodgedInActor = target;
		attacker->stanceFlags |= IS_HOLDING_LODGED_WEAPON;
		target->stanceFlags |= IS_STUCK_WITH_LODGED_WEAPON;
		
		if (attacker == player) {
			showMessage(10, "The weapon becomes lodged!", NULL);
		}
	} else if (attacker->stanceFlags & IS_MOVING) {
		considerKnockback(attacker, target, attackDamage, percentageAttackDamage);
	}
	
	return 0;
}

int stab(character *attacker, character *target, item *weapon) {
	character *player = getPlayer();
	int attackerStrength = getActorStrength(attacker);
	int attackerLevel = getActorLevel(attacker);
	int attackerLuck = getActorLuck(attacker);
	float recoveryTime = getAttackSpeedOfWeapon(weapon) * getActorStability(attacker);
	int weaponDamage = getRandomIntWithMean(0, weapon->statDamage, weapon->statDamage + (2 + attackerLevel)) + attacker->statStabCount;

	attackerLuck = getRandomIntWithMean(0, attackerLuck, clip(attackerLuck - 1, 0, attackerLuck));

	float targetOpenness = getTargetOpenness(target);
	int lowerDamageValue = attackerStrength;
	int upperDamageValue = (attackerStrength + 2) + attackerLevel + attackerLuck;
	int damageMean = (upperDamageValue * (1.3 * targetOpenness)) + weaponDamage;
	float attackDamage, percentageAttackDamage, movementDamageBonus = .0f;

	if (attacker->stanceFlags & IS_STABBING) {
		movementDamageBonus = (10.f - ((float)getActorSpeed(attacker))) / 10.f;
		movementDamageBonus *= attackerStrength;

		printf("Extra movement damage: %f\n", movementDamageBonus);
	}

	lowerDamageValue += (int)(movementDamageBonus + .5f);
	upperDamageValue += (int)(movementDamageBonus + .5f);
	damageMean += (int)(movementDamageBonus + .5f);
	attacker->statStabCount ++;

	setStance(attacker, IS_STABBING);
	setDelay(attacker, recoveryTime);

	attackDamage = getRandomIntWithMean(lowerDamageValue, upperDamageValue, damageMean);
	percentageAttackDamage = attackDamage / upperDamageValue;

	float attackDamageBeforeDefense = attackDamage;

	if ((attacker->traitFlags & TORCH_ATTACK_PENALTY && !actorGetItemWithFlag(attacker, IS_TORCH_HOLDER) && attacker->itemLight)) {
		attackDamage /= 2;

		attacker->itemLight->fuel -= 25;
	}

	printf("lower=%i, upper=%i, perc=%f, actual=%f\n", lowerDamageValue, upperDamageValue, percentageAttackDamage, attackDamage);

	if (performDamage(attacker, target, attackDamage, percentageAttackDamage)) {
		return 1;
	}

	if (attackDamageBeforeDefense <= upperDamageValue * .85 && getRandomFloat(0.f, 1.f) > (attackDamageBeforeDefense / (upperDamageValue * .85)) / (upperDamageValue * .85)) {
		weapon->itemFlags |= IS_LODGED;
		weapon->lodgedInActor = target;
		attacker->stanceFlags |= IS_HOLDING_LODGED_WEAPON;
		target->stanceFlags |= IS_STUCK_WITH_LODGED_WEAPON;

		if (attacker == player) {
			showMessage(10, "The weapon becomes lodged!", NULL);
		}
	} else if (attacker->stanceFlags & IS_MOVING) {
		considerKnockback(attacker, target, attackDamage, percentageAttackDamage);
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
	} else if (attackerWeapon->itemFlags & IS_DAGGER) {
		return stab(attacker, target, attackerWeapon);
	}
	
	printf("Something terrible has happened.\n");
	
	return 0;
}
