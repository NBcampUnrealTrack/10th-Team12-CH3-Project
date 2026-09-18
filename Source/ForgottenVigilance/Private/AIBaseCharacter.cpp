#include "AIBaseCharacter.h"
#include "AIBaseController.h"
#include "HealthComponent.h"
#include "ScoreOnDeathComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BlackboardComponent.h"

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

	PreviousHealth = HealthComponent->GetCurrentHealth();

	HealthComponent->OnHealthChanged.AddDynamic(this, &AAIBaseCharacter::HandleHealthChanged);

	HealthComponent->OnDeath.AddDynamic(this, &AAIBaseCharacter::HandleDeath);
	
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
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
		if (HitMontage)
		{
			UE_LOG(LogTemp, Warning, TEXT("AI HIT!"));
			PlayAnimMontage(HitMontage);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("HitMontage is NULL!"));
		}
	}

	UE_LOG(LogTemp, Warning,
	    TEXT("AI Health Changed: %.1f / %.1f"),
	    CurrentHealth,
	    MaxHealth);

	PreviousHealth = CurrentHealth;
}

void AAIBaseCharacter::HandleDeath(AActor* DeadOwner)
{
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
