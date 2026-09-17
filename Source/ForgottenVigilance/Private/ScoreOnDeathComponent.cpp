#include "ScoreOnDeathComponent.h"

#include "ForgottenGameState.h"
#include "HealthComponent.h"

namespace
{
constexpr int32 DefaultScoreValue = 100;
}

UScoreOnDeathComponent::UScoreOnDeathComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	ScoreValue = DefaultScoreValue;
	bScoreRegistered = false;
}

void UScoreOnDeathComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* OwnerActor = GetOwner();

	if (!OwnerActor)
	{
		return;
	}

	UHealthComponent* HealthComponent = OwnerActor->FindComponentByClass<UHealthComponent>();

	if (!HealthComponent)
	{
		return;
	}

	HealthComponent->OnDeath.AddDynamic(this, &UScoreOnDeathComponent::HandleDeath);
}

void UScoreOnDeathComponent::HandleDeath(AActor* DeadOwner)
{
	if (bScoreRegistered)
	{
		return;
	}

	AForgottenGameState* ForgottenGameState = GetWorld()->GetGameState<AForgottenGameState>();

	if (!ForgottenGameState)
	{
		return;
	}

	bScoreRegistered = true;
	ForgottenGameState->RegisterEnemyKill(ScoreValue);
}
