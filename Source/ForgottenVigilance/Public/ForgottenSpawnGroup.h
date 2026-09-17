#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ForgottenGameState.h"
#include "ForgottenSpawnGroup.generated.h"

class UBoxComponent;

USTRUCT(BlueprintType)
struct FForgottenSpawnEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	TSubclassOf<AActor> EnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	int32 Count = 1;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGroupCleared, AForgottenSpawnGroup*, ClearedGroup);

UCLASS()
class FORGOTTENVIGILANCE_API AForgottenSpawnGroup : public AActor
{
	GENERATED_BODY()

public:
	AForgottenSpawnGroup();

	void ActivateGroup();
	bool IsCleared() const;
	bool IsActivated() const;
	int32 GetAliveCount() const;

	UPROPERTY(BlueprintAssignable, Category = "Spawn")
	FOnGroupCleared OnGroupCleared;

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void HandlePhaseChanged(EForgottenPhase NewPhase);

	UFUNCTION()
	void HandleEnemyDeath(AActor* DeadOwner);

	void SpawnEntries();
	void SpawnSingleEnemy(TSubclassOf<AActor> EnemyClass);
	void MarkCleared();
	FVector GetRandomPointInBox() const;
	float GetCapsuleHalfHeight(TSubclassOf<AActor> EnemyClass) const;
	bool FindSpawnLocation(TSubclassOf<AActor> EnemyClass, FVector& OutLocation) const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn", meta = (AllowPrivateAccess = true))
	TArray<FForgottenSpawnEntry> SpawnTable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn", meta = (AllowPrivateAccess = true))
	EForgottenPhase ActivationPhase;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawn", meta = (AllowPrivateAccess = true))
	TObjectPtr<UBoxComponent> SpawnBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn", meta = (AllowPrivateAccess = true))
	bool bAdvancePhaseOnCleared;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn", meta = (AllowPrivateAccess = true))
	EForgottenPhase NextPhaseOnCleared;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawn", meta = (AllowPrivateAccess = true))
	int32 AliveCount;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawn", meta = (AllowPrivateAccess = true))
	bool bActivated;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawn", meta = (AllowPrivateAccess = true))
	bool bCleared;
};
