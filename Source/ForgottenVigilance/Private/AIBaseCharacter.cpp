#include "AIBaseCharacter.h"
#include "AIBaseController.h"
#include "HealthComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AAIBaseCharacter::AAIBaseCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	AIControllerClass = AAIBaseController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->MaxWalkSpeed = WalkSpeed;
	Movement->bOrientRotationToMovement = true;
	Movement->RotationRate = FRotator(0.0f, 540.0f, 0.0f);

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
}

void AAIBaseCharacter::SetMovementSpeed(float NewSpeed)
{
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->MaxWalkSpeed = NewSpeed;
		UE_LOG(LogTemp, Warning, TEXT("[Sparta] Speed changed: %.1f"), NewSpeed);
	}
}

void AAIBaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	HealthComponent->OnDeath.AddDynamic(this, &AAIBaseCharacter::HandleDeath);
}

void AAIBaseCharacter::HandleDeath(AActor* DeadOwner)
{
	//죽을때 처리할 것들 여기에
	this->Destroy();
}

bool AAIBaseCharacter::CanMeleeAttack() const
{
	return !bIsAttackOnCooldown;
}

void AAIBaseCharacter::PerformMeleeAttack(AActor* TargetActor)
{
	if (!TargetActor || !CanMeleeAttack())
	{
		return;
	}

	const float Distance = FVector::Dist(GetActorLocation(), TargetActor->GetActorLocation());
	if (Distance > AttackRange)
	{
		return;
	}

	UGameplayStatics::ApplyDamage(TargetActor, AttackDamage, GetController(), this, nullptr);

	bIsAttackOnCooldown = true;
	GetWorldTimerManager().SetTimer(
	    AttackCooldownTimer,
	    this,
	    &AAIBaseCharacter::ResetAttackCooldown,
	    AttackCooldown,
	    false);
}

void AAIBaseCharacter::ResetAttackCooldown()
{
	bIsAttackOnCooldown = false;
}
