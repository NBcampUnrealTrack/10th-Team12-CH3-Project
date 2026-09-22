#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ForgottenGameState.h"
#include "ForgottenLever.generated.h"

class AForgottenSpawnGroup;
class USceneComponent;
class USphereComponent;
class UStaticMeshComponent;
class UTextRenderComponent;

UCLASS()
class FORGOTTENVIGILANCE_API AForgottenLever : public AActor
{
	GENERATED_BODY()

public:
	AForgottenLever();

	virtual void Tick(float DeltaTime) override;

	void TryInteract();
	bool CanInteract() const;
	bool IsActivated() const;

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void HandleOverlapBegin(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void HandleOverlapEnd(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	UFUNCTION()
	void HandleGuardianGroupCleared(AForgottenSpawnGroup* ClearedGroup);

	void ActivateLever();
	void UpdatePromptVisibility();
	void SetOutlineEnabled(bool bEnabled);
	void StartHandleRotation();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lever", meta = (AllowPrivateAccess = true))
	TObjectPtr<USceneComponent> LeverRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lever", meta = (AllowPrivateAccess = true))
	TObjectPtr<UStaticMeshComponent> BodyMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lever", meta = (AllowPrivateAccess = true))
	TObjectPtr<USceneComponent> HandlePivot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lever", meta = (AllowPrivateAccess = true))
	TObjectPtr<UStaticMeshComponent> HandleMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lever", meta = (AllowPrivateAccess = true))
	TObjectPtr<USphereComponent> TriggerSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lever", meta = (AllowPrivateAccess = true))
	TObjectPtr<UTextRenderComponent> PromptText;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Lever", meta = (AllowPrivateAccess = true))
	TObjectPtr<AForgottenSpawnGroup> GuardianSpawnGroup;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Lever", meta = (AllowPrivateAccess = true))
	TObjectPtr<AForgottenSpawnGroup> ReturnSpawnGroup;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Lever", meta = (AllowPrivateAccess = true))
	bool bActivateGuardianOnApproach;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lever|Animation", meta = (AllowPrivateAccess = true))
	FRotator ActivatedRotationOffset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lever|Animation", meta = (AllowPrivateAccess = true))
	float RotateDuration;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lever", meta = (AllowPrivateAccess = true))
	bool bGuardianDefeated;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lever", meta = (AllowPrivateAccess = true))
	bool bPlayerInRange;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lever", meta = (AllowPrivateAccess = true))
	bool bActivated;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lever|Animation", meta = (AllowPrivateAccess = true))
	FRotator HandleStartRotation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lever|Animation", meta = (AllowPrivateAccess = true))
	float RotateElapsed;
};