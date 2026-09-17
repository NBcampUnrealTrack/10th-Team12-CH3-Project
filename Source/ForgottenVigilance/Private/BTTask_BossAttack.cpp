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
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("BossAttack: No BB or AIOwner"));
		return EBTNodeResult::Failed;
	}

	AAIBossCharacter* BossCharacter = Cast<AAIBossCharacter>(AIOwner->GetPawn());
	AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(BossTargetActorKey));

	if (!BossCharacter)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("BossAttack: Pawn is not BossCharacter"));
		return EBTNodeResult::Failed;
	}

	if (!TargetActor)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("BossAttack: No TargetActor"));
		return EBTNodeResult::Failed;
	}

	const float MontageDuration = BossCharacter->ExecuteAttackPattern(TargetActor);

	if (MontageDuration <= AttackZeroThreshold)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("BossAttack: Duration 0"));
		return EBTNodeResult::Failed;
	}

	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green,
		FString::Printf(TEXT("BossAttack: playing %.2fs"), MontageDuration));

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