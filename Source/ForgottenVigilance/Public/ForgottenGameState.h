#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "ForgottenGameState.generated.h"

class AOptimusPrimePlayerController;

UENUM(BlueprintType)
enum class EForgottenPhase : uint8
{
	Exploring,
	FinalBoss,
	Cleared,
	Failed
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScoreChanged, int32, NewScore);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnKillCountChanged, int32, NewKillCount);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectiveProgressChanged, int32, CurrentProgress, int32, RequiredProgress);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPhaseChanged, EForgottenPhase, NewPhase);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRemainingTimeChanged, float, RemainingTime);

UCLASS()
class FORGOTTENVIGILANCE_API AForgottenGameState : public AGameState
{
	GENERATED_BODY()

public:
	AForgottenGameState();

	void RegisterEnemyKill(int32 ScoreAmount);
	void AdvanceObjectiveProgress();
	void EnterPhase(EForgottenPhase NewPhase);
	void NotifyRemainingTime(float NewRemainingTime);
	bool IsObjectiveCompleted() const;
	int32 GetScore() const;
	int32 GetKillCount() const;
	int32 GetObjectiveProgress() const;
	int32 GetRequiredObjectiveProgress() const;
	EForgottenPhase GetPhase() const;

	UPROPERTY(BlueprintAssignable, Category = "Forgotten")
	FOnScoreChanged OnScoreChanged;

	UPROPERTY(BlueprintAssignable, Category = "Forgotten")
	FOnKillCountChanged OnKillCountChanged;

	UPROPERTY(BlueprintAssignable, Category = "Forgotten")
	FOnObjectiveProgressChanged OnObjectiveProgressChanged;

	UPROPERTY(BlueprintAssignable, Category = "Forgotten")
	FOnPhaseChanged OnPhaseChanged;

	UPROPERTY(BlueprintAssignable, Category = "Forgotten")
	FOnRemainingTimeChanged OnRemainingTimeChanged;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Forgotten", meta = (AllowPrivateAccess = true))
	int32 RequiredObjectiveProgress;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Forgotten", meta = (AllowPrivateAccess = true))
	int32 Score;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Forgotten", meta = (AllowPrivateAccess = true))
	int32 KillCount;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Forgotten", meta = (AllowPrivateAccess = true))
	int32 ObjectiveProgress;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Forgotten", meta = (AllowPrivateAccess = true))
	EForgottenPhase Phase;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player", meta = (AllowPrivateAccess = true))
	TObjectPtr<AOptimusPrimePlayerController> PlayerController;
};
