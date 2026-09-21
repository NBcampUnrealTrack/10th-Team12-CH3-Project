#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AIBaseCharacter.generated.h"

class UHealthComponent;
class UScoreOnDeathComponent;
class UAnimMontage;

UCLASS()
class FORGOTTENVIGILANCE_API AAIBaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AAIBaseCharacter();
	void SetMovementSpeed(float NewSpeed);

	UPROPERTY(EditAnywhere, Category = "AI")
	float WalkSpeed = 300.0f;

	UPROPERTY(EditAnywhere, Category = "AI")
	float RunSpeed = 600.0f;
	
	UPROPERTY(EditAnywhere, Category = "AI") 
	bool bLoseTargetOnHit = true;
	
	UPROPERTY(EditAnywhere, Category = "AI")
	bool bStunOnHit = true;

	UPROPERTY(EditAnywhere, Category = "AI")
	float StunDuration = 1.0f;

	UPROPERTY(EditAnywhere, Category = "AI")
	float DeadDestoryTime = 3.0f;

	UPROPERTY(VisibleAnywhere, Category = "AI")
	FVector SpawnLocation;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Animation")
	TObjectPtr<UAnimMontage> HitMontage;

	UPROPERTY(EditAnywhere, Category = "Animation")
	TObjectPtr<UAnimMontage> DeathMontage;

private:
	UFUNCTION()
	void HandleHealthChanged(float CurrentHealth, float MaxHealth);

	UFUNCTION()
	void HandleDeath(AActor* DeadOwner);
	
	UFUNCTION()
	void ClearStun();

	UPROPERTY(EditDefaultsOnly, Category = "Health")
	TObjectPtr<UHealthComponent> HealthComponent;

	UPROPERTY(EditDefaultsOnly, Category = "GameState")
	TObjectPtr<UScoreOnDeathComponent> ScoreOnDeathComponent;

	float PreviousHealth = 0.0f;
	
	FTimerHandle StunTimerHandle;
};
