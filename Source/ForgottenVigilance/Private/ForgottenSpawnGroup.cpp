#include "ForgottenSpawnGroup.h"

#include "HealthComponent.h"
#include "Components/BillboardComponent.h"
#include "Engine/World.h"
#include "NavigationSystem.h"

namespace
{
constexpr float DefaultSpawnRadius = 800.0f;
constexpr int32 ZeroCount = 0;
constexpr int32 SingleStep = 1;
}

AForgottenSpawnGroup::AForgottenSpawnGroup()
{
	PrimaryActorTick.bCanEverTick = false;

	RootBillboard = CreateDefaultSubobject<UBillboardComponent>(TEXT("RootBillboard"));
	SetRootComponent(RootBillboard);

	ActivationPhase = EForgottenPhase::Route1Combat;
	SpawnRadius = DefaultSpawnRadius;
	bAdvancePhaseOnCleared = false;
	NextPhaseOnCleared = EForgottenPhase::Route1Combat;
	AliveCount = ZeroCount;
	bActivated = false;
	bCleared = false;
}

void AForgottenSpawnGroup::BeginPlay()
{
	Super::BeginPlay();

	AForgottenGameState* ForgottenGameState = GetWorld()->GetGameState<AForgottenGameState>();

	if (!ForgottenGameState)
	{
		return;
	}

	ForgottenGameState->OnPhaseChanged.AddDynamic(this, &AForgottenSpawnGroup::HandlePhaseChanged);

	if (ForgottenGameState->GetPhase() != ActivationPhase)
	{
		return;
	}

	ActivateGroup();
}

void AForgottenSpawnGroup::HandlePhaseChanged(EForgottenPhase NewPhase)
{
	if (NewPhase != ActivationPhase)
	{
		return;
	}

	ActivateGroup();
}

void AForgottenSpawnGroup::ActivateGroup()
{
	if (bActivated)
	{
		return;
	}

	bActivated = true;

	SpawnEntries();

	if (AliveCount > ZeroCount)
	{
		return;
	}

	MarkCleared();
}

void AForgottenSpawnGroup::SpawnEntries()
{
	for (const FForgottenSpawnEntry& SpawnEntry : SpawnTable)
	{
		if (!SpawnEntry.EnemyClass)
		{
			continue;
		}

		for (int32 SpawnIndex = 0; SpawnIndex < SpawnEntry.Count; ++SpawnIndex)
		{
			SpawnSingleEnemy(SpawnEntry.EnemyClass);
		}
	}
}

void AForgottenSpawnGroup::SpawnSingleEnemy(TSubclassOf<AActor> EnemyClass)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AActor* SpawnedEnemy = GetWorld()->SpawnActor<AActor>(
		EnemyClass,
		FindSpawnLocation(),
		GetActorRotation(),
		SpawnParams);

	if (!SpawnedEnemy)
	{
		return;
	}

	UHealthComponent* EnemyHealth = SpawnedEnemy->FindComponentByClass<UHealthComponent>();

	if (!EnemyHealth)
	{
		return;
	}

	EnemyHealth->OnDeath.AddDynamic(this, &AForgottenSpawnGroup::HandleEnemyDeath);
	AliveCount += SingleStep;
}

void AForgottenSpawnGroup::HandleEnemyDeath(AActor* DeadOwner)
{
	if (bCleared)
	{
		return;
	}

	AliveCount = FMath::Max(AliveCount - SingleStep, ZeroCount);

	if (AliveCount > ZeroCount)
	{
		return;
	}

	MarkCleared();
}

void AForgottenSpawnGroup::MarkCleared()
{
	if (bCleared)
	{
		return;
	}

	bCleared = true;
	OnGroupCleared.Broadcast(this);

	if (!bAdvancePhaseOnCleared)
	{
		return;
	}

	AForgottenGameState* ForgottenGameState = GetWorld()->GetGameState<AForgottenGameState>();

	if (!ForgottenGameState)
	{
		return;
	}

	ForgottenGameState->EnterPhase(NextPhaseOnCleared);
}

FVector AForgottenSpawnGroup::FindSpawnLocation() const
{
	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());

	if (!NavSystem)
	{
		return GetActorLocation();
	}

	FNavLocation NavLocation;

	if (!NavSystem->GetRandomReachablePointInRadius(GetActorLocation(), SpawnRadius, NavLocation))
	{
		return GetActorLocation();
	}

	return NavLocation.Location;
}

bool AForgottenSpawnGroup::IsCleared() const
{
	return bCleared;
}

bool AForgottenSpawnGroup::IsActivated() const
{
	return bActivated;
}

int32 AForgottenSpawnGroup::GetAliveCount() const
{
	return AliveCount;
}
