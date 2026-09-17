#include "AIBaseCharacter.h"
#include "AIBaseController.h"
#include "HealthComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

AAIBaseCharacter::AAIBaseCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	AIControllerClass = AAIBaseController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->MaxWalkSpeed = WalkSpeed;
	Movement->bOrientRotationToMovement = true;
	Movement->RotationRate = FRotator(0.0f, 540.0f, 0.0f);

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
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
}

void AAIBaseCharacter::HandleHealthChanged(float CurrentHealth, float MaxHealth)
{
	if (CurrentHealth < PreviousHealth && CurrentHealth > 0.0f)
	{
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
	if (DeathMontage)
	{
		PlayAnimMontage(DeathMontage);
	}

	GetCharacterMovement()->DisableMovement();

	SetLifeSpan(3.0f);
	this->Destroy();
}
