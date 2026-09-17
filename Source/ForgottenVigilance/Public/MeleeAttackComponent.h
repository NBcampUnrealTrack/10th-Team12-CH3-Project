#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MeleeAttackComponent.generated.h"

class UAnimMontage;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FORGOTTENVIGILANCE_API UMeleeAttackComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMeleeAttackComponent();

	bool CanAttack() const;
	void PerformAttack(AActor* TargetActor);
	void ApplyCachedDamage();

	UPROPERTY(EditAnywhere, Category = "Attack")
	float AttackDamage = 20.0f;

	UPROPERTY(EditAnywhere, Category = "Attack")
	float AttackRange = 200.0f;

	UPROPERTY(EditAnywhere, Category = "Attack")
	float AttackCooldown = 1.5f;

	UPROPERTY(EditAnywhere, Category = "Attack")
	TObjectPtr<UAnimMontage> AttackMontage;

private:
	void ResetAttackCooldown();
	void ApplyDamage(AActor* TargetActor);

	FTimerHandle AttackCooldownTimer;
	bool bIsAttackOnCooldown = false;

	TWeakObjectPtr<AActor> CachedTarget;
};
