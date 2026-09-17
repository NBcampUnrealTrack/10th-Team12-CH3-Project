#include "ForgottenGameState.h"

#include "OptimusPrimePlayerController.h"

namespace
{
constexpr int32 DefaultRequiredObjectiveProgress = 2;
constexpr int32 ZeroCount = 0;
constexpr int32 SingleStep = 1;
}

AForgottenGameState::AForgottenGameState()
{
	RequiredObjectiveProgress = DefaultRequiredObjectiveProgress;
	Score = ZeroCount;
	KillCount = ZeroCount;
	ObjectiveProgress = ZeroCount;
	Phase = EForgottenPhase::Route1Combat;
	PlayerController = nullptr;
}

void AForgottenGameState::BeginPlay()
{
	Super::BeginPlay();

	PlayerController = GetWorld()->GetFirstPlayerController<AOptimusPrimePlayerController>();

	if (!PlayerController)
	{
		return;
	}

	const FString PureLevelName = GetWorld()->GetOuter()->GetName();

	if (PureLevelName.Contains(TEXT("TitleMap")))
	{
		PlayerController->ShowMainMenu();
	}
	else if (PureLevelName.Contains(TEXT("MainLevel")))
	{
		PlayerController->ShowHUD();
	}
}

void AForgottenGameState::RegisterEnemyKill(int32 ScoreAmount)
{
	KillCount += SingleStep;
	OnKillCountChanged.Broadcast(KillCount);

	if (ScoreAmount == ZeroCount)
	{
		return;
	}

	Score += ScoreAmount;
	OnScoreChanged.Broadcast(Score);
}

void AForgottenGameState::AdvanceObjectiveProgress()
{
	if (IsObjectiveCompleted())
	{
		return;
	}

	ObjectiveProgress = FMath::Min(ObjectiveProgress + SingleStep, RequiredObjectiveProgress);
	OnObjectiveProgressChanged.Broadcast(ObjectiveProgress, RequiredObjectiveProgress);
}

void AForgottenGameState::EnterPhase(EForgottenPhase NewPhase)
{
	if (Phase == NewPhase)
	{
		return;
	}

	Phase = NewPhase;
	OnPhaseChanged.Broadcast(Phase);
}

void AForgottenGameState::NotifyRemainingTime(float NewRemainingTime)
{
	OnRemainingTimeChanged.Broadcast(NewRemainingTime);
}

bool AForgottenGameState::IsObjectiveCompleted() const
{
	return ObjectiveProgress >= RequiredObjectiveProgress;
}

int32 AForgottenGameState::GetScore() const
{
	return Score;
}

int32 AForgottenGameState::GetKillCount() const
{
	return KillCount;
}

int32 AForgottenGameState::GetObjectiveProgress() const
{
	return ObjectiveProgress;
}

int32 AForgottenGameState::GetRequiredObjectiveProgress() const
{
	return RequiredObjectiveProgress;
}

EForgottenPhase AForgottenGameState::GetPhase() const
{
	return Phase;
}
