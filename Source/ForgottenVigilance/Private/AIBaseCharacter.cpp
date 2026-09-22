#include "AIBaseCharacter.h"
#include "AIBaseController.h"
#include "HealthComponent.h"
#include "ScoreOnDeathComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "TimerManager.h"
#include "Animation/AnimInstance.h"
#include "Components/TextRenderComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/WidgetComponent.h"
#include "Components/Image.h"
#include "Blueprint/UserWidget.h"

AAIBaseCharacter::AAIBaseCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	AIControllerClass = AAIBaseController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->MaxWalkSpeed = WalkSpeed;
	Movement->bOrientRotationToMovement = true;
	Movement->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	Movement->bUseRVOAvoidance = true;

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	ScoreOnDeathComponent = CreateDefaultSubobject<UScoreOnDeathComponent>(TEXT("ScoreOnDeathComponent"));
	
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	AlertWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("AlertWidget"));
	AlertWidget->SetupAttachment(GetRootComponent());
	AlertWidget->SetRelativeLocation(FVector(0.0f, 0.0f, AlertHeightOffset));
	AlertWidget->SetWidgetSpace(EWidgetSpace::Screen);
	AlertWidget->SetDrawSize(AlertDrawSize);
	AlertWidget->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AlertWidget->SetVisibility(false);
}

void AAIBaseCharacter::SetMovementSpeed(float NewSpeed)
{
	UCharacterMovementComponent* Movement = GetCharacterMovement();

	if (!Movement || FMath::IsNearlyEqual(Movement->MaxWalkSpeed, NewSpeed))
	{
		return;
	}

	Movement->MaxWalkSpeed = NewSpeed;
	UE_LOG(LogTemp, Warning, TEXT("[Sparta] Speed changed: %.1f"), NewSpeed);
}

void AAIBaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	SpawnLocation = GetActorLocation();
	
	PreviousHealth = HealthComponent->GetCurrentHealth();

	HealthComponent->OnHealthChanged.AddDynamic(this, &AAIBaseCharacter::HandleHealthChanged);

	HealthComponent->OnDeath.AddDynamic(this, &AAIBaseCharacter::HandleDeath);
	
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	
	AlertWidget->SetRelativeLocation(FVector(0.0f, 0.0f, AlertHeightOffset));
	AlertWidget->SetDrawSize(AlertDrawSize);
	UpdateAlertVisual();
}

void AAIBaseCharacter::HandleHealthChanged(float CurrentHealth, float MaxHealth)
{

	if (CurrentHealth < PreviousHealth && CurrentHealth > 0.0f)
	{
		if (bLoseTargetOnHit)
		{
			APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

			if (PlayerPawn)
			{
				AAIBaseController* AIController = Cast<AAIBaseController>(GetController());

				if (PlayerPawn && AIController)
				{
					UBlackboardComponent* Blackboard = AIController->GetBlackboardComponent();

					if (Blackboard)
					{
						Blackboard->SetValueAsVector(TEXT("LastKnownLocation"), PlayerPawn->GetActorLocation());

						Blackboard->ClearValue(TEXT("TargetActor"));

						Blackboard->SetValueAsBool(TEXT("IsChasing"), false);
					}
				}
			}
		}
		if (bStunOnHit)
		{
			ApplyStun(StunDuration);
		}
	}

	UE_LOG(LogTemp, Warning,
	    TEXT("AI Health Changed: %.1f / %.1f"),
	    CurrentHealth,
	    MaxHealth);

	PreviousHealth = CurrentHealth;
}

void AAIBaseCharacter::ApplyStun(float Duration)
{
	if (Duration <= 0.0f)
	{
		return;
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (HitMontage)
	{
		if (AnimInstance && !AnimInstance->Montage_IsPlaying(HitMontage))
		{
			PlayAnimMontage(HitMontage);
		}
	}
	else
	{
		StopAnimMontage();
	}

	AAIBaseController* StunController = Cast<AAIBaseController>(GetController());

	if (StunController)
	{
		StunController->StopMovement();

		UBlackboardComponent* StunBlackboard = StunController->GetBlackboardComponent();

		if (StunBlackboard)
		{
			StunBlackboard->SetValueAsBool(TEXT("IsStunned"), true);
		}
	}

	GetWorldTimerManager().SetTimer(
		StunTimerHandle,
		this,
		&AAIBaseCharacter::ClearStun,
		Duration,
		false);
}

void AAIBaseCharacter::ClearStun()
{
	AAIBaseController* StunController = Cast<AAIBaseController>(GetController());

	if (!StunController)
	{
		return;
	}

	UBlackboardComponent* StunBlackboard = StunController->GetBlackboardComponent();

	if (!StunBlackboard)
	{
		return;
	}

	StunBlackboard->SetValueAsBool(TEXT("IsStunned"), false);
}

void AAIBaseCharacter::HandleDeath(AActor* DeadOwner)
{
	SetAlertChasing(false);
	
	AAIBaseController* AIController = Cast<AAIBaseController>(GetController());

	if (AIController)
	{
		AIController->StopMovement();
		if (AIController->BrainComponent)
		{
			AIController->BrainComponent->StopLogic(TEXT("Dead"));
		}
	}
	

	GetCharacterMovement()->DisableMovement();

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (DeathMontage)
	{
		PlayAnimMontage(DeathMontage);
		DeadDestoryTime = DeathMontage->GetPlayLength() - 0.28f;
		SetLifeSpan(DeadDestoryTime);
	}
	else
	{
		SetLifeSpan(DeadDestoryTime);
	}
}

void AAIBaseCharacter::SetAlertChasing(bool bIsChasing)
{
	if (bAlertChasing == bIsChasing)
	{
		return;
	}

	bAlertChasing = bIsChasing;

	if (!bIsChasing)
	{
		bAlertAttacking = false;
		GetWorldTimerManager().ClearTimer(AttackAlertTimerHandle);
	}

	UpdateAlertVisual();
}

void AAIBaseCharacter::SetAlertAttacking()
{
	bAlertAttacking = true;
	UpdateAlertVisual();

	GetWorldTimerManager().SetTimer(
		AttackAlertTimerHandle,
		this,
		&AAIBaseCharacter::ClearAttackAlert,
		AttackAlertDuration,
		false);
}

void AAIBaseCharacter::ClearAttackAlert()
{
	bAlertAttacking = false;
	UpdateAlertVisual();
}

void AAIBaseCharacter::UpdateAlertVisual()
{
	if (!AlertWidget)
	{
		return;
	}

	const bool bShowAlert = bAlertChasing || bAlertAttacking;

	AlertWidget->SetVisibility(bShowAlert);

	if (!bShowAlert)
	{
		return;
	}

	UUserWidget* AlertUserWidget = AlertWidget->GetUserWidgetObject();

	if (!AlertUserWidget)
	{
		return;
	}

	UImage* AlertImage = Cast<UImage>(AlertUserWidget->GetWidgetFromName(TEXT("AlertImage")));

	if (!AlertImage)
	{
		return;
	}

	UTexture2D* AlertTexture = bAlertAttacking ? AttackAlertTexture : ChaseAlertTexture;

	if (!AlertTexture)
	{
		return;
	}

	AlertImage->SetBrushFromTexture(AlertTexture, true);
}

