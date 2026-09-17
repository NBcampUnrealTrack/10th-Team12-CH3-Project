#include "ForgottenLever.h"

#include "ForgottenSpawnGroup.h"
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

	PromptText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("PromptText"));
	PromptText->SetupAttachment(LeverMesh);
	PromptText->SetRelativeLocation(FVector(0.0f, 0.0f, PromptHeightOffset));
	PromptText->SetHorizontalAlignment(EHTA_Center);
	PromptText->SetWorldSize(PromptWorldSize);
	PromptText->SetText(FText::FromString(TEXT("[E]")));
	PromptText->SetVisibility(false);

	GuardianSpawnGroup = nullptr;
	GuardianPhase = EForgottenPhase::Route1Guardian;
	ReturnPhase = EForgottenPhase::Route1Return;
	bGuardianDefeated = false;
	bPlayerInRange = false;
	bActivated = false;
}

void AForgottenLever::BeginPlay()
{
	Super::BeginPlay();

	TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &AForgottenLever::HandleOverlapBegin);
	TriggerSphere->OnComponentEndOverlap.AddDynamic(this, &AForgottenLever::HandleOverlapEnd);

	if (!GuardianSpawnGroup)
	{
		bGuardianDefeated = true;
		return;
	}

	GuardianSpawnGroup->OnGroupCleared.AddDynamic(this, &AForgottenLever::HandleGuardianGroupCleared);
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

	if (bActivated || bGuardianDefeated)
	{
		return;
	}

	AForgottenGameState* ForgottenGameState = GetWorld()->GetGameState<AForgottenGameState>();

	if (!ForgottenGameState)
	{
		return;
	}

	ForgottenGameState->EnterPhase(GuardianPhase);
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

	AForgottenGameState* ForgottenGameState = GetWorld()->GetGameState<AForgottenGameState>();

	if (!ForgottenGameState)
	{
		return;
	}

	ForgottenGameState->AdvanceObjectiveProgress();

	if (ForgottenGameState->IsObjectiveCompleted())
	{
		ForgottenGameState->EnterPhase(EForgottenPhase::FinalBoss);
		return;
	}

	ForgottenGameState->EnterPhase(ReturnPhase);
}

void AForgottenLever::UpdatePromptVisibility()
{
	PromptText->SetVisibility(CanInteract());
}

bool AForgottenLever::IsActivated() const
{
	return bActivated;
}
