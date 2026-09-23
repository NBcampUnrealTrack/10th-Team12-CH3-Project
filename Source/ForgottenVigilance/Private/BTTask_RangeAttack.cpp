#include "BTTask_RangeAttack.h"

#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "AIRangeCharacter.h"
#include "AIRangeWeaponComponent.h"

UBTTask_RangeAttack::UBTTask_RangeAttack()
{
	NodeName = TEXT("Ranged Attack");

	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_RangeAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();

	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	AAIRangeCharacter* AICharacter = Cast<AAIRangeCharacter>(AIController->GetPawn());

	if (!AICharacter)
	{
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();

	if (!BlackboardComp)
	{
		return EBTNodeResult::Failed;
	}

	AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TEXT("TargetActor")));

	if (!TargetActor)
	{
		return EBTNodeResult::Failed;
	}

	FVector Direction = TargetActor->GetActorLocation() - AICharacter->GetActorLocation();

	Direction.Z = 0.0f;

	if (Direction.IsNearlyZero())
	{
		return EBTNodeResult::Failed;
	}

	FRotator TargetRotation = Direction.Rotation();

	return EBTNodeResult::InProgress;
}

void UBTTask_RangeAttack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AIController = OwnerComp.GetAIOwner();

	if (!AIController)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	AAIRangeCharacter* AICharacter = Cast<AAIRangeCharacter>(AIController->GetPawn());

	if (!AICharacter)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();

	if (!BlackboardComp)
	{
		return;
	}

	AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TEXT("TargetActor")));

	if (!TargetActor)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	FVector Direction = TargetActor->GetActorLocation() - AICharacter->GetActorLocation();

	Direction.Z = 0.0f;

	if (Direction.IsNearlyZero())
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	FRotator TargetRotation = Direction.Rotation();

	FRotator CurrentRoatation = AICharacter->GetActorRotation();

	FRotator NewRotation = FMath::RInterpTo(CurrentRoatation, TargetRotation, DeltaSeconds, 5.0f);

	AICharacter->SetActorRotation(NewRotation);

	if (NewRotation.Equals(TargetRotation, 1.0f))
	{
		UAIRangeWeaponComponent* Weapon = AICharacter->FindComponentByClass<UAIRangeWeaponComponent>();

		if (Weapon)
		{
			AAIBaseCharacter* AlertCharacter = Cast<AAIBaseCharacter>(AICharacter);

			if (AlertCharacter)
			{
				AlertCharacter->SetAlertAttacking();
			}

			Weapon->FireGun();
		}

		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}
