#include "AIRangeCharacter.h"
#include "AIRangeWeaponComponent.h"


AAIRangeCharacter::AAIRangeCharacter()
{
	AIWeaponComponent = CreateDefaultSubobject<UAIWeaponComponent>(TEXT("AIWeaponComponent"));
}

