#include "ForgottenGameMode.h"

#include "ForgottenGameState.h"
#include "HealthComponent.h"
#include "OptimusPrimePlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

namespace
{
constexpr float DefaultMissionTimeLimit = 600.0f;
constexpr float MissionTickInterval = 1.0f;
constexpr float BindRetryInterval = 0.2f;
constexpr float TimeZeroThreshold = 0.0f;
}

AForgottenGameMode::AForgottenGameMode()
{
	MissionTimeLimit = DefaultMissionTimeLimit;
	RemainingTime = DefaultMissionTimeLimit;
	bGameFinished = false;
}

void AForgottenGameMode::BeginPlay()
{
	Super::BeginPlay();

	RemainingTime = MissionTimeLimit;
	bGameFinished = false;

	BindPlayerHealth();

	GetWorldTimerManager().SetTimer(
		MissionTimerHandle,
		this,
		&AForgottenGameMode::TickMissionTimer,
		MissionTickInterval,
		true);
}

void AForgottenGameMode::BindPlayerHealth()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	if (!PlayerPawn)
	{
		GetWorldTimerManager().SetTimer(
			BindRetryHandle,
			this,
			&AForgottenGameMode::BindPlayerHealth,
			BindRetryInterval,
			false);
		return;
	}

	UHealthComponent* HealthComponent = PlayerPawn->FindComponentByClass<UHealthComponent>();

	if (!HealthComponent)
	{
		return;
	}

	HealthComponent->OnDeath.AddDynamic(this, &AForgottenGameMode::HandlePlayerDeath);
}

void AForgottenGameMode::TickMissionTimer()
{
	if (bGameFinished)
	{
		return;
	}

	RemainingTime = FMath::Max(RemainingTime - MissionTickInterval, TimeZeroThreshold);

	AForgottenGameState* ForgottenGameState = GetForgottenGameState();

	if (ForgottenGameState)
	{
		ForgottenGameState->NotifyRemainingTime(RemainingTime);
	}

	if (RemainingTime > TimeZeroThreshold)
	{
		return;
	}

	FinishGame(false);
}

void AForgottenGameMode::HandlePlayerDeath(AActor* DeadOwner)
{
	FinishGame(false);
}

void AForgottenGameMode::NotifyFinalBossDefeated()
{
	FinishGame(true);
}

void AForgottenGameMode::FinishGame(bool bCleared)
{
	if (bGameFinished)
	{
		return;
	}

	bGameFinished = true;
	GetWorldTimerManager().ClearTimer(MissionTimerHandle);

	AForgottenGameState* ForgottenGameState = GetForgottenGameState();

	if (ForgottenGameState)
	{
		ForgottenGameState->EnterPhase(bCleared ? EForgottenPhase::Cleared : EForgottenPhase::Failed);
	}

	AOptimusPrimePlayerController* ForgottenController =
		GetWorld()->GetFirstPlayerController<AOptimusPrimePlayerController>();

	if (!ForgottenController)
	{
		return;
	}

	ForgottenController->ShowGameOver();
}

AForgottenGameState* AForgottenGameMode::GetForgottenGameState() const
{
	return GetWorld()->GetGameState<AForgottenGameState>();
}
