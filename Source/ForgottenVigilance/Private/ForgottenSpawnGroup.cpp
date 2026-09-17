#include "ForgottenSpawnGroup.h"

#include "HealthComponent.h"
#include "Engine/World.h"
#include "NavigationSystem.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"

namespace
{
constexpr int32 SpawnZeroCount = 0;
constexpr int32 SpawnSingleStep = 1;
constexpr float DefaultBoxExtentXY = 800.0f;
constexpr float DefaultBoxExtentZ = 200.0f;
constexpr float NavProjectExtentXY = 200.0f;
constexpr float NavProjectExtentZ = 500.0f;
constexpr int32 MaxSpawnAttempts = 10;
constexpr float SpawnZeroHeight = 0.0f;
}

AForgottenSpawnGroup::AForgottenSpawnGroup()
{
	PrimaryActorTick.bCanEverTick = false;

	SpawnBox = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnBox"));
	SetRootComponent(SpawnBox);
	SpawnBox->SetBoxExtent(FVector(DefaultBoxExtentXY, DefaultBoxExtentXY, DefaultBoxExtentZ));
	SpawnBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SpawnBox->SetHiddenInGame(true);

	ActivationPhase = EForgottenPhase::Route1Combat;
	bAdvancePhaseOnCleared = false;
	NextPhaseOnCleared = EForgottenPhase::Route1Combat;
	AliveCount = SpawnZeroCount;
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

	if (AliveCount > SpawnZeroCount)
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
	FVector SpawnLocation = GetActorLocation();
	FindSpawnLocation(EnemyClass, SpawnLocation);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AActor* SpawnedEnemy = GetWorld()->SpawnActor<AActor>(
		EnemyClass,
		SpawnLocation,
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
	AliveCount += SpawnSingleStep;
}

void AForgottenSpawnGroup::HandleEnemyDeath(AActor* DeadOwner)
{
	if (bCleared)
	{
		return;
	}

	AliveCount = FMath::Max(AliveCount - SpawnSingleStep, SpawnZeroCount);

	if (AliveCount > SpawnZeroCount)
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

FVector AForgottenSpawnGroup::GetRandomPointInBox() const
{
	const FVector BoxExtent = SpawnBox->GetScaledBoxExtent();

	const FVector LocalOffset(
		FMath::FRandRange(-BoxExtent.X, BoxExtent.X),
		FMath::FRandRange(-BoxExtent.Y, BoxExtent.Y),
		FMath::FRandRange(-BoxExtent.Z, BoxExtent.Z));

	return SpawnBox->GetComponentTransform().TransformPositionNoScale(LocalOffset);
}

float AForgottenSpawnGroup::GetCapsuleHalfHeight(TSubclassOf<AActor> EnemyClass) const
{
	if (!EnemyClass)
	{
		return SpawnZeroHeight;
	}

	const ACharacter* EnemyDefault = Cast<ACharacter>(EnemyClass->GetDefaultObject());

	if (!EnemyDefault || !EnemyDefault->GetCapsuleComponent())
	{
		return SpawnZeroHeight;
	}

	return EnemyDefault->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
}

bool AForgottenSpawnGroup::FindSpawnLocation(TSubclassOf<AActor> EnemyClass, FVector& OutLocation) const
{
	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());

	if (!NavSystem)
	{
		return false;
	}

	const FVector ProjectExtent(NavProjectExtentXY, NavProjectExtentXY, NavProjectExtentZ);
	const float HalfHeight = GetCapsuleHalfHeight(EnemyClass);

	for (int32 AttemptIndex = 0; AttemptIndex < MaxSpawnAttempts; ++AttemptIndex)
	{
		FNavLocation NavLocation;

		if (!NavSystem->ProjectPointToNavigation(GetRandomPointInBox(), NavLocation, ProjectExtent))
		{
			continue;
		}

		OutLocation = NavLocation.Location + FVector(SpawnZeroHeight, SpawnZeroHeight, HalfHeight);
		return true;
	}

	return false;
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
