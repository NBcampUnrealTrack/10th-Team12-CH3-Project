#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AIRangeWeaponComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FORGOTTENVIGILANCE_API UAIRangeWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAIRangeWeaponComponent();

	void FireGun();

private:
	bool AITraceForHit(FHitResult& OutHit) const;

	UPROPERTY(EditAnywhere, Category = "Weapon")
	float AIGunDamage = 50.0f;

	UPROPERTY(EditAnywhere, Category = "Weapon")
	float AIGunAttackRange = 1000.0f;


		
};
