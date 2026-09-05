#include "ForgottenGameState.h"

namespace
{
constexpr int32 DefaultRequiredObjectiveProgress = 3;
constexpr int32 ZeroCount = 0;
}

AForgottenGameState::AForgottenGameState()
{
	RequiredObjectiveProgress = DefaultRequiredObjectiveProgress;
	Score = ZeroCount;
	KillCount = ZeroCount;
	ObjectiveProgress = ZeroCount;
	BossPhase = EBossPhase::None;
}

void AForgottenGameState::AddScore(int32 ScoreAmount)
{
	if (ScoreAmount == ZeroCount)
	{
		return;
	}

	Score += ScoreAmount;
	OnScoreChanged.Broadcast(Score);
}

void AForgottenGameState::RegisterEnemyKill(int32 ScoreAmount)
{
	KillCount += 1;
	OnKillCountChanged.Broadcast(KillCount);

	AddScore(ScoreAmount);
}

void AForgottenGameState::AdvanceObjectiveProgress(int32 ProgressAmount)
{
	if (ProgressAmount <= ZeroCount)
	{
		return;
	}

	if (IsObjectiveCompleted())
	{
		return;
	}

	ObjectiveProgress = FMath::Min(ObjectiveProgress + ProgressAmount, RequiredObjectiveProgress);
	OnObjectiveProgressChanged.Broadcast(ObjectiveProgress, RequiredObjectiveProgress);
}

void AForgottenGameState::EnterBossPhase(EBossPhase NewPhase)
{
	if (BossPhase == NewPhase)
	{
		return;
	}

	BossPhase = NewPhase;
	OnBossPhaseChanged.Broadcast(BossPhase);
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

EBossPhase AForgottenGameState::GetBossPhase() const
{
	return BossPhase;
}
