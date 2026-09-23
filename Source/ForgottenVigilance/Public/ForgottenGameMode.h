#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "ForgottenGameMode.generated.h"

class AForgottenGameState;

UCLASS()
class FORGOTTENVIGILANCE_API AForgottenGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	AForgottenGameMode();

	void NotifyFinalBossDefeated();

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void HandlePlayerDeath(AActor* DeadOwner);

	void BindPlayerHealth();
	void TickMissionTimer();
	void FinishGame(bool bCleared);
	AForgottenGameState* GetForgottenGameState() const;
	void ScheduleFreeze();
	void FreezeWorld();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rule", meta = (AllowPrivateAccess = true))
	float MissionTimeLimit;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rule", meta = (AllowPrivateAccess = true))
	float RemainingTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rule", meta = (AllowPrivateAccess = true))
	bool bGameFinished;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rule", meta = (AllowPrivateAccess = true))
	float FreezeDelay;

	FTimerHandle MissionTimerHandle;
	FTimerHandle BindRetryHandle;
	FTimerHandle FreezeTimerHandle;
};
