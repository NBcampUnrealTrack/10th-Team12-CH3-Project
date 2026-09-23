#include "BTService_UpdateBossState.h"

#include "AIBossCharacter.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

namespace
{
const FName StateTargetActorKey(TEXT("TargetActor"));
const FName InMeleeRangeKey(TEXT("IsInMeleeRange"));
const FName InDashRangeKey(TEXT("IsInDashRange"));
const FName SecondPhaseKey(TEXT("IsSecondPhase"));
constexpr float BossServiceInterval = 0.15f;
}

UBTService_UpdateBossState::UBTService_UpdateBossState()
{
	NodeName = TEXT("Update Boss State");
	Interval = BossServiceInterval;
	RandomDeviation = 0.0f;
}

void UBTService_UpdateBossState::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	AAIController* AIOwner = OwnerComp.GetAIOwner();

	if (!BlackboardComp || !AIOwner)
	{
		return;
	}

	AAIBossCharacter* BossCharacter = Cast<AAIBossCharacter>(AIOwner->GetPawn());

	if (!BossCharacter)
	{
		return;
	}

	BlackboardComp->SetValueAsBool(SecondPhaseKey, BossCharacter->GetBossPhase() == EBossPhase::Second);

	AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(StateTargetActorKey));

	if (!TargetActor)
	{
		BlackboardComp->SetValueAsBool(InMeleeRangeKey, false);
		BlackboardComp->SetValueAsBool(InDashRangeKey, false);
		return;
	}

	const float DistanceToTarget = FVector::Dist(
		BossCharacter->GetActorLocation(),
		TargetActor->GetActorLocation());

	BlackboardComp->SetValueAsBool(InMeleeRangeKey, DistanceToTarget <= BossCharacter->GetMeleeRange());
	BlackboardComp->SetValueAsBool(InDashRangeKey, DistanceToTarget <= BossCharacter->GetDashRange());
}
