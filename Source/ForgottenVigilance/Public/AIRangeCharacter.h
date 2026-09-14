#pragma once

#include "CoreMinimal.h"
#include "AIBaseCharacter.h"
#include "GameFramework/Actor.h"
#include "AIRangeCharacter.generated.h"

class UAIWeaponComponent;

UCLASS()
class FORGOTTENVIGILANCE_API AAIRangeCharacter : public AAIBaseCharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AAIRangeCharacter();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Weapon", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAIWeaponComponent> AIWeaponComponent;
};
