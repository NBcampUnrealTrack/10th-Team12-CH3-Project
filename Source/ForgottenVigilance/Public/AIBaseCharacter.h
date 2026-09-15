#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AIBaseCharacter.generated.h"

class UHealthComponent;

UCLASS()
class FORGOTTENVIGILANCE_API AAIBaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AAIBaseCharacter();
	void SetMovementSpeed(float NewSpeed);

	bool CanMeleeAttack() const;
	void PerformMeleeAttack(AActor* TargetActor);

	UPROPERTY(EditAnywhere, Category = "AI")
	float WalkSpeed = 300.0f;

	UPROPERTY(EditAnywhere, Category = "AI")
	float RunSpeed = 600.0f;
	
protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Attack")
	float AttackDamage = 20.0f;

	UPROPERTY(EditAnywhere, Category = "Attack")
	float AttackRange = 200.0f;

	UPROPERTY(EditAnywhere, Category = "Attack")
	float AttackCooldown = 1.5f;

private:
	UFUNCTION()
	void HandleDeath(AActor* DeadOwner);

	void ResetAttackCooldown();

	UPROPERTY(EditDefaultsOnly, Category = "Health")
	TObjectPtr<UHealthComponent> HealthComponent;

	FTimerHandle AttackCooldownTimer;
	bool bIsAttackOnCooldown = false;
};
