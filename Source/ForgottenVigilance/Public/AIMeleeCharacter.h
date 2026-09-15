#pragma once

#include "CoreMinimal.h"
#include "AIBaseCharacter.h"
#include "AIMeleeCharacter.generated.h"

class UMeleeAttackComponent;

UCLASS()
class FORGOTTENVIGILANCE_API AAIMeleeCharacter : public AAIBaseCharacter
{
	GENERATED_BODY()

public:
	AAIMeleeCharacter();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Attack", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMeleeAttackComponent> MeleeAttackComponent;
};
