#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "ForgottenGameState.generated.h"

UENUM(BlueprintType)
enum class EBossPhase : uint8
{
	None,
	First,
	Second,
	Defeated
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScoreChanged, int32, NewScore);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnKillCountChanged, int32, NewKillCount);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectiveProgressChanged, int32, CurrentProgress, int32, RequiredProgress);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBossPhaseChanged, EBossPhase, NewPhase);

UCLASS()
class FORGOTTENVIGILANCE_API AForgottenGameState : public AGameState
{
	GENERATED_BODY()

public:
	void AddScore(int32 ScoreAmount);
	void RegisterEnemyKill(int32 ScoreAmount);
	void AdvanceObjectiveProgress(int32 ProgressAmount);
	void EnterBossPhase(EBossPhase NewPhase);
	bool IsObjectiveCompleted() const;
	int32 GetScore() const;
	int32 GetKillCount() const;
	EBossPhase GetBossPhase() const;

	UPROPERTY(BlueprintAssignable, Category = Survival)
	FOnScoreChanged OnScoreChanged;

	UPROPERTY(BlueprintAssignable, Category = Survival)
	FOnKillCountChanged OnKillCountChanged;

	UPROPERTY(BlueprintAssignable, Category = Survival)
	FOnObjectiveProgressChanged OnObjectiveProgressChanged;

	UPROPERTY(BlueprintAssignable, Category = Survival)
	FOnBossPhaseChanged OnBossPhaseChanged;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Survival, meta = (AllowPrivateAccess = true))
	int32 RequiredObjectiveProgress;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Survival, meta = (AllowPrivateAccess = true))
	int32 Score;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Survival, meta = (AllowPrivateAccess = true))
	int32 KillCount;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Survival, meta = (AllowPrivateAccess = true))
	int32 ObjectiveProgress;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Survival, meta = (AllowPrivateAccess = true))
	EBossPhase BossPhase;
};
