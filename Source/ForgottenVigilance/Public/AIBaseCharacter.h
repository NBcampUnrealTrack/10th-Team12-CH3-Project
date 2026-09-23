#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AIBaseCharacter.generated.h"

class UHealthComponent;
class UScoreOnDeathComponent;
class UAnimMontage;
class UWidgetComponent;

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
	bool bLoseTargetOnHit = false;

	UPROPERTY(EditAnywhere, Category = "AI")
	bool bStunOnHit = true;

	UPROPERTY(EditAnywhere, Category = "AI")
	float StunDuration = 1.0f;

	UPROPERTY(EditAnywhere, Category = "AI")
	float DeadDestoryTime = 3.0f;

	UPROPERTY(VisibleAnywhere, Category = "AI")
	FVector SpawnLocation;


	void SetAlertChasing(bool bIsChasing);
	void SetAlertAttacking();

protected:
	virtual void BeginPlay() override;

	void ApplyStun(float Duration);

	UPROPERTY(EditAnywhere, Category = "Animation")
	TObjectPtr<UAnimMontage> HitMontage;

	UPROPERTY(EditAnywhere, Category = "Animation")
	TObjectPtr<UAnimMontage> DeathMontage;

	UPROPERTY(EditAnywhere, Category = "Alert")
	TObjectPtr<UTexture2D> ChaseAlertTexture;

	UPROPERTY(EditAnywhere, Category = "Alert")
	TObjectPtr<UTexture2D> AttackAlertTexture;

	UPROPERTY(EditAnywhere, Category = "Alert")
	float AlertHeightOffset = 120.0f;

	UPROPERTY(EditAnywhere, Category = "Alert")
	FVector2D AlertDrawSize = FVector2D(64.0f, 64.0f);

	UPROPERTY(EditAnywhere, Category = "Alert")
	float AttackAlertDuration = 1.2f;

private:
	UFUNCTION()
	void HandleHealthChanged(float CurrentHealth, float MaxHealth);

	UFUNCTION()
	void HandleDeath(AActor* DeadOwner);

	UFUNCTION()
	void ClearStun();

	UFUNCTION()
	void ClearAttackAlert();

	void UpdateAlertVisual();

	UPROPERTY(EditDefaultsOnly, Category = "Health")
	TObjectPtr<UHealthComponent> HealthComponent;

	UPROPERTY(EditDefaultsOnly, Category = "GameState")
	TObjectPtr<UScoreOnDeathComponent> ScoreOnDeathComponent;

	UPROPERTY(VisibleAnywhere, Category = "Alert")
	TObjectPtr<UWidgetComponent> AlertWidget;

	bool bAlertChasing = false;

	bool bAlertAttacking = false;

	FTimerHandle AttackAlertTimerHandle;

	float PreviousHealth = 0.0f;

	FTimerHandle StunTimerHandle;

};
