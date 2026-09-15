#include "AIRangeCharacter.h"
#include "AIRangeWeaponComponent.h"


AAIRangeCharacter::AAIRangeCharacter()
{
	AIWeaponComponent = CreateDefaultSubobject<UAIRangeWeaponComponent>(TEXT("AIRangeWeaponComponent"));
}

