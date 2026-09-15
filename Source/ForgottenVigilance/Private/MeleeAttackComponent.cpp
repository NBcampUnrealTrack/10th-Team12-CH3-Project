#include "MeleeAttackComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UMeleeAttackComponent::UMeleeAttackComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UMeleeAttackComponent::CanAttack() const
{
	return !bIsAttackOnCooldown;
}

void UMeleeAttackComponent::PerformAttack(AActor* TargetActor)
{
	AActor* OwnerActor = GetOwner();

	if (!OwnerActor || !TargetActor || !CanAttack())
	{
		return;
	}

	const float Distance = FVector::Dist(OwnerActor->GetActorLocation(), TargetActor->GetActorLocation());
	if (Distance > AttackRange)
	{
		return;
	}

	APawn* OwnerPawn = Cast<APawn>(OwnerActor);
	UGameplayStatics::ApplyDamage(
	    TargetActor,
	    AttackDamage,
	    OwnerPawn ? OwnerPawn->GetController() : nullptr,
	    OwnerActor,
	    nullptr);

	bIsAttackOnCooldown = true;
	GetWorld()->GetTimerManager().SetTimer(
	    AttackCooldownTimer,
	    this,
	    &UMeleeAttackComponent::ResetAttackCooldown,
	    AttackCooldown,
	    false);
}

void UMeleeAttackComponent::ResetAttackCooldown()
{
	bIsAttackOnCooldown = false;
}
