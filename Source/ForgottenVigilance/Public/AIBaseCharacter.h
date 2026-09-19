#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AIBaseCharacter.generated.h"

class UHealthComponent;
class UScoreOnDeathComponent;
class UAnimMontage;
class UBrainComponent;
class AAIController;

UCLASS()
class FORGOTTENVIGILANCE_API AAIBaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AAIBaseCharacter();
	void SetMovementSpeed(float NewSpeed);
	void SetReconSuppressed(bool bSuppressed);
	bool IsReconSuppressed() const { return bReconSuppressed; }

	UPROPERTY(EditAnywhere, Category = "AI")
	float WalkSpeed = 300.0f;

	UPROPERTY(EditAnywhere, Category = "AI")
	float RunSpeed = 600.0f;
	
	UPROPERTY(EditAnywhere, Category = "AI") 
	bool bLoseTargetOnHit = true;

	UPROPERTY(EditAnywhere, Category = "AI")
	float DeadDestoryTime = 3.0f;

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

	UPROPERTY(EditDefaultsOnly, Category = "Health")
	TObjectPtr<UHealthComponent> HealthComponent;

	UPROPERTY(EditDefaultsOnly, Category = "GameState")
	TObjectPtr<UScoreOnDeathComponent> ScoreOnDeathComponent;

	float PreviousHealth = 0.0f;

	// 정찰 시 변경한 상태만 복구하며, 사망 후에는 AI/이동을 재개하지 않습니다.
	bool bReconSuppressed = false;
	bool bReconPausedBrain = false;
	bool bReconControllerTickEnabled = false;
	bool bReconUseControllerRotationYaw = false;
	uint8 ReconPreviousMovementMode = 0;
	uint8 ReconPreviousCustomMovementMode = 0;
	TWeakObjectPtr<UBrainComponent> ReconPausedBrain;
	TWeakObjectPtr<AAIController> ReconAIController;
};
