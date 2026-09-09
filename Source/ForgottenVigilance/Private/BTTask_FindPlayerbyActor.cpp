#include "BTTask_FindPlayerbyActor.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_FindPlayerbyActor::UBTTask_FindPlayerbyActor()
{
	NodeName = TEXT("Find Player Location");
}

EBTNodeResult::Type UBTTask_FindPlayerbyActor::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
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

	BlackboardComp->SetValueAsVector(TEXT("PlayerVector"), TargetActor->GetActorLocation());

	return EBTNodeResult::Succeeded;
}

