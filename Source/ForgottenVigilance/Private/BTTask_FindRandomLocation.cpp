#include "BTTask_FindRandomLocation.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "AIBaseCharacter.h"


UBTTask_FindRandomLocation::UBTTask_FindRandomLocation()
{
	NodeName = TEXT("Find Random Location");

	LocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_FindRandomLocation, LocationKey));
}


EBTNodeResult::Type UBTTask_FindRandomLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	APawn* MyPawn = AIController->GetPawn();
	if (!MyPawn)
	{
		return EBTNodeResult::Failed;
	}

	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSystem)
	{
		return EBTNodeResult::Failed;
	}

	AAIBaseCharacter* AIChracter = Cast<AAIBaseCharacter>(MyPawn);

	if (!AIChracter)
	{
		return EBTNodeResult::Failed;
	}

	FNavLocation RandomLocation;
	bool bFound = NavSystem->GetRandomReachablePointInRadius(
	    AIChracter->SpawnLocation,
	    SearchRadius,
	    RandomLocation);

	if (!bFound)
	{
		UE_LOG(LogTemp, Warning, TEXT("[FindRandom]갈곳 찾지못함"));
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp)
	{
		return EBTNodeResult::Failed;
	}

	BlackboardComp->SetValueAsVector(LocationKey.SelectedKeyName, RandomLocation.Location);
	UE_LOG(LogTemp, Log, TEXT("[FindRandom] 새로운 목적지: %s"), *RandomLocation.Location.ToString());

	return EBTNodeResult::Succeeded;
}
