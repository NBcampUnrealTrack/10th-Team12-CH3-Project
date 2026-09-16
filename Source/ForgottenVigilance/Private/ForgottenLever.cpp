#include "ForgottenLever.h"

#include "HealthComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

namespace
{
constexpr float DefaultTriggerRadius = 400.0f;
const FName PlayerTagName(TEXT("Player"));
}

AForgottenLever::AForgottenLever()
{
	PrimaryActorTick.bCanEverTick = false;

	LeverMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeverMesh"));
	SetRootComponent(LeverMesh);
	LeverMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
	TriggerSphere->SetupAttachment(LeverMesh);
	TriggerSphere->SetSphereRadius(DefaultTriggerRadius);
	TriggerSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	GuardianClass = nullptr;
	GuardianSpawnPoint = nullptr;
	GuardianPhase = EForgottenPhase::Route1Guardian;
	ReturnPhase = EForgottenPhase::Route1Return;
	SpawnedGuardian = nullptr;
	bGuardianSpawned = false;
	bActivated = false;
	bGuardianDefeated = false;
	bPlayerInRange = false;
}

void AForgottenLever::BeginPlay()
{
	Super::BeginPlay();

	TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &AForgottenLever::HandleOverlapBegin);
	TriggerSphere->OnComponentEndOverlap.AddDynamic(this, &AForgottenLever::HandleOverlapEnd);
}

void AForgottenLever::HandleOverlapBegin(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!OtherActor || !OtherActor->ActorHasTag(PlayerTagName))
	{
		return;
	}

	bPlayerInRange = true;

	if (bGuardianSpawned || bActivated)
	{
		return;
	}

	SpawnGuardian();
}

void AForgottenLever::HandleOverlapEnd(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	if (!OtherActor || !OtherActor->ActorHasTag(PlayerTagName))
	{
		return;
	}

	bPlayerInRange = false;
}

void AForgottenLever::SpawnGuardian()
{
	if (!GuardianClass)
	{
		ActivateLever();
		return;
	}

	const FVector SpawnLocation = GuardianSpawnPoint
		? GuardianSpawnPoint->GetActorLocation()
		: GetActorLocation();
	const FRotator SpawnRotation = GuardianSpawnPoint
		? GuardianSpawnPoint->GetActorRotation()
		: GetActorRotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	SpawnedGuardian = GetWorld()->SpawnActor<AActor>(GuardianClass, SpawnLocation, SpawnRotation, SpawnParams);

	if (!SpawnedGuardian)
	{
		ActivateLever();
		return;
	}

	bGuardianSpawned = true;

	AForgottenGameState* ForgottenGameState = GetWorld()->GetGameState<AForgottenGameState>();

	if (ForgottenGameState)
	{
		ForgottenGameState->EnterPhase(GuardianPhase);
	}

	UHealthComponent* GuardianHealth = SpawnedGuardian->FindComponentByClass<UHealthComponent>();

	if (!GuardianHealth)
	{
		return;
	}

	GuardianHealth->OnDeath.AddDynamic(this, &AForgottenLever::HandleGuardianDeath);
}

void AForgottenLever::HandleGuardianDeath(AActor* DeadOwner)
{
	bGuardianDefeated = true;
}

void AForgottenLever::ActivateLever()
{
	if (bActivated)
	{
		return;
	}

	bActivated = true;

	AForgottenGameState* ForgottenGameState = GetWorld()->GetGameState<AForgottenGameState>();

	if (!ForgottenGameState)
	{
		return;
	}

	ForgottenGameState->AdvanceObjectiveProgress();
	ForgottenGameState->EnterPhase(ReturnPhase);
}

bool AForgottenLever::IsActivated() const
{
	return bActivated;
}

bool AForgottenLever::CanInteract() const
{
	return bPlayerInRange && bGuardianDefeated && !bActivated;
}

void AForgottenLever::TryInteract()
{
	if (!CanInteract())
	{
		return;
	}

	ActivateLever();
}
