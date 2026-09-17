#include "MeleeAttackComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
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

	bIsAttackOnCooldown = true;
	GetWorld()->GetTimerManager().SetTimer(
	    AttackCooldownTimer,
	    this,
	    &UMeleeAttackComponent::ResetAttackCooldown,
	    AttackCooldown,
	    false);

	ACharacter* OwnerCharacter = Cast<ACharacter>(OwnerActor);
	if (AttackMontage && OwnerCharacter && OwnerCharacter->PlayAnimMontage(AttackMontage) > 0.0f)
	{
		// 몽타주 안의 AnimNotify(ApplyCachedDamage)가 타격 타이밍에 데미지를 적용
		CachedTarget = TargetActor;
		return;
	}

	// 몽타주가 없으면 예전처럼 즉시 데미지 적용 (애니메이션 준비 전 폴백)
	ApplyDamage(TargetActor);
}

void UMeleeAttackComponent::ApplyCachedDamage()
{
	if (AActor* TargetActor = CachedTarget.Get())
	{
		ApplyDamage(TargetActor);
	}
}

void UMeleeAttackComponent::ApplyDamage(AActor* TargetActor)
{
	AActor* OwnerActor = GetOwner();

	if (!OwnerActor || !TargetActor)
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
}

void UMeleeAttackComponent::ResetAttackCooldown()
{
	bIsAttackOnCooldown = false;
}
