#include "ForgottenLever.h"

#include "ForgottenSpawnGroup.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/World.h"

namespace
{
constexpr float DefaultTriggerRadius = 350.0f;
constexpr float PromptHeightOffset = 150.0f;
constexpr float PromptWorldSize = 48.0f;
const FName PlayerTagName(TEXT("Player"));
constexpr int32 LeverOutlineStencilValue = 1;
constexpr float DefaultRotateDuration = 0.6f;
constexpr float HalfTurnDegrees = 180.0f;
constexpr float RotateZeroThreshold = 0.0f;
constexpr float RotateAlphaMin = 0.0f;
constexpr float RotateAlphaMax = 1.0f;
constexpr float RotateEaseExponent = 2.0f;
}

AForgottenLever::AForgottenLever()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	LeverRoot = CreateDefaultSubobject<USceneComponent>(TEXT("LeverRoot"));
	SetRootComponent(LeverRoot);

	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetupAttachment(LeverRoot);
	BodyMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BodyMesh->SetCustomDepthStencilValue(LeverOutlineStencilValue);
	BodyMesh->SetRenderCustomDepth(false);

	HandlePivot = CreateDefaultSubobject<USceneComponent>(TEXT("HandlePivot"));
	HandlePivot->SetupAttachment(LeverRoot);

	HandleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HandleMesh"));
	HandleMesh->SetupAttachment(HandlePivot);
	HandleMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	HandleMesh->SetCustomDepthStencilValue(LeverOutlineStencilValue);
	HandleMesh->SetRenderCustomDepth(false);

	TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
	TriggerSphere->SetupAttachment(LeverRoot);
	TriggerSphere->SetSphereRadius(DefaultTriggerRadius);
	TriggerSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	PromptText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("PromptText"));
	PromptText->SetupAttachment(LeverRoot);
	PromptText->SetRelativeLocation(FVector(0.0f, 0.0f, PromptHeightOffset));
	PromptText->SetHorizontalAlignment(EHTA_Center);
	PromptText->SetWorldSize(PromptWorldSize);
	PromptText->SetText(FText::FromString(TEXT("[E]")));
	PromptText->SetVisibility(false);

	GuardianSpawnGroup = nullptr;
	ReturnSpawnGroup = nullptr;
	bActivateGuardianOnApproach = false;
	ActivatedRotationOffset = FRotator(0.0f, 0.0f, HalfTurnDegrees);
	RotateDuration = DefaultRotateDuration;
	bGuardianDefeated = false;
	bPlayerInRange = false;
	bActivated = false;
	HandleStartRotation = FRotator::ZeroRotator;
	RotateElapsed = 0.0f;
}

void AForgottenLever::BeginPlay()
{
	Super::BeginPlay();

	TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &AForgottenLever::HandleOverlapBegin);
	TriggerSphere->OnComponentEndOverlap.AddDynamic(this, &AForgottenLever::HandleOverlapEnd);

	UpdatePromptVisibility();

	if (!GuardianSpawnGroup)
	{
		bGuardianDefeated = true;
		return;
	}

	GuardianSpawnGroup->OnGroupCleared.AddDynamic(this, &AForgottenLever::HandleGuardianGroupCleared);
}

void AForgottenLever::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RotateElapsed = FMath::Min(RotateElapsed + DeltaTime, RotateDuration);

	const float RotateAlpha = RotateDuration > RotateZeroThreshold
		                          ? RotateElapsed / RotateDuration
		                          : RotateAlphaMax;

	const float EasedAlpha = FMath::InterpEaseInOut(RotateAlphaMin, RotateAlphaMax, RotateAlpha, RotateEaseExponent);

	HandlePivot->SetRelativeRotation(HandleStartRotation + ActivatedRotationOffset * EasedAlpha);

	if (RotateAlpha < RotateAlphaMax)
	{
		return;
	}

	SetActorTickEnabled(false);
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
	UpdatePromptVisibility();

	if (bActivated || bGuardianDefeated || !bActivateGuardianOnApproach)
	{
		return;
	}

	if (!IsValid(GuardianSpawnGroup))
	{
		return;
	}

	GuardianSpawnGroup->ActivateGroup();
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
	UpdatePromptVisibility();
}

void AForgottenLever::HandleGuardianGroupCleared(AForgottenSpawnGroup* ClearedGroup)
{
	bGuardianDefeated = true;
	UpdatePromptVisibility();
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

void AForgottenLever::ActivateLever()
{
	if (bActivated)
	{
		return;
	}

	bActivated = true;
	UpdatePromptVisibility();
	StartHandleRotation();

	if (IsValid(ReturnSpawnGroup))
	{
		ReturnSpawnGroup->ActivateGroup();
	}

	AForgottenGameState* ForgottenGameState = GetWorld()->GetGameState<AForgottenGameState>();

	if (!ForgottenGameState)
	{
		return;
	}

	ForgottenGameState->AdvanceObjectiveProgress();
}

void AForgottenLever::StartHandleRotation()
{
	HandleStartRotation = HandlePivot->GetRelativeRotation();
	RotateElapsed = 0.0f;
	SetActorTickEnabled(true);
}

void AForgottenLever::UpdatePromptVisibility()
{
	const bool bInteractable = CanInteract();

	PromptText->SetVisibility(bInteractable);
	SetOutlineEnabled(bInteractable);
}

void AForgottenLever::SetOutlineEnabled(bool bEnabled)
{
	BodyMesh->SetRenderCustomDepth(bEnabled);
	HandleMesh->SetRenderCustomDepth(bEnabled);
}

bool AForgottenLever::IsActivated() const
{
	return bActivated;
}
