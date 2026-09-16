#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ForgottenGameState.h"
#include "ForgottenLever.generated.h"

class UStaticMeshComponent;
class USphereComponent;

UCLASS()
class FORGOTTENVIGILANCE_API AForgottenLever : public AActor
{
	GENERATED_BODY()

public:
	AForgottenLever();

	bool IsActivated() const;
	void TryInteract();
	bool CanInteract() const;

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
	void HandleGuardianDeath(AActor* DeadOwner);

	void SpawnGuardian();
	void ActivateLever();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lever", meta = (AllowPrivateAccess = true))
	TObjectPtr<UStaticMeshComponent> LeverMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lever", meta = (AllowPrivateAccess = true))
	TObjectPtr<USphereComponent> TriggerSphere;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lever", meta = (AllowPrivateAccess = true))
	TSubclassOf<AActor> GuardianClass;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Lever", meta = (AllowPrivateAccess = true))
	TObjectPtr<AActor> GuardianSpawnPoint;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Lever", meta = (AllowPrivateAccess = true))
	EForgottenPhase GuardianPhase;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Lever", meta = (AllowPrivateAccess = true))
	EForgottenPhase ReturnPhase;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lever", meta = (AllowPrivateAccess = true))
	TObjectPtr<AActor> SpawnedGuardian;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lever", meta = (AllowPrivateAccess = true))
	bool bGuardianSpawned;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lever", meta = (AllowPrivateAccess = true))
	bool bActivated;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lever", meta = (AllowPrivateAccess = true))
	bool bGuardianDefeated;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lever", meta = (AllowPrivateAccess = true))
	bool bPlayerInRange;
};