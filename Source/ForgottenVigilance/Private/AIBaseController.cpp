#include "AIBaseController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"

AAIBaseController::AAIBaseController()
{
}

void AAIBaseController::StartBehaviorTree()
{
	if (BehaviorTreeAsset)
	{
		RunBehaviorTree(BehaviorTreeAsset);

		UE_LOG(LogTemp, Warning, TEXT("[ICE AGE] bt started"));
	}
}

void AAIBaseController::BeginPlay()
{
	Super::BeginPlay();


	StartBehaviorTree();

}
