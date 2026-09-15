#include "AIMeleeCharacter.h"
#include "MeleeAttackComponent.h"

AAIMeleeCharacter::AAIMeleeCharacter()
{
	MeleeAttackComponent = CreateDefaultSubobject<UMeleeAttackComponent>(TEXT("MeleeAttackComponent"));
}
