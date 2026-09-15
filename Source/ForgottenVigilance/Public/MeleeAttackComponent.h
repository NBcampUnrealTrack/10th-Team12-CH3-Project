#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MeleeAttackComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FORGOTTENVIGILANCE_API UMeleeAttackComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMeleeAttackComponent();

	bool CanAttack() const;
	void PerformAttack(AActor* TargetActor);

	UPROPERTY(EditAnywhere, Category = "Attack")
	float AttackDamage = 20.0f;

	UPROPERTY(EditAnywhere, Category = "Attack")
	float AttackRange = 200.0f;

	UPROPERTY(EditAnywhere, Category = "Attack")
	float AttackCooldown = 1.5f;

private:
	void ResetAttackCooldown();

	FTimerHandle AttackCooldownTimer;
	bool bIsAttackOnCooldown = false;
};
