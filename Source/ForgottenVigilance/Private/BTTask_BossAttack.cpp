#include "BTTask_BossAttack.h"

#include "AIBossCharacter.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

namespace
{
const FName BossTargetActorKey(TEXT("TargetActor"));
constexpr float AttackZeroThreshold = 0.0f;
}

UBTTask_BossAttack::UBTTask_BossAttack()
{
	NodeName = TEXT("Boss Attack");
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_BossAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	AAIController* AIOwner = OwnerComp.GetAIOwner();

	if (!BlackboardComp || !AIOwner)
	{
		return EBTNodeResult::Failed;
	}

	AAIBossCharacter* BossCharacter = Cast<AAIBossCharacter>(AIOwner->GetPawn());
	AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(BossTargetActorKey));

	if (!BossCharacter || !TargetActor)
	{
		return EBTNodeResult::Failed;
	}

	const float MontageDuration = BossCharacter->ExecuteAttackPattern(TargetActor);

	if (MontageDuration <= AttackZeroThreshold)
	{
		return EBTNodeResult::Failed;
	}

	FBTBossAttackMemory* AttackMemory = reinterpret_cast<FBTBossAttackMemory*>(NodeMemory);
	AttackMemory->RemainingTime = MontageDuration;

	return EBTNodeResult::InProgress;
}

void UBTTask_BossAttack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	FBTBossAttackMemory* AttackMemory = reinterpret_cast<FBTBossAttackMemory*>(NodeMemory);
	AttackMemory->RemainingTime -= DeltaSeconds;

	if (AttackMemory->RemainingTime > AttackZeroThreshold)
	{
		return;
	}

	FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
}

uint16 UBTTask_BossAttack::GetInstanceMemorySize() const
{
	return sizeof(FBTBossAttackMemory);
}