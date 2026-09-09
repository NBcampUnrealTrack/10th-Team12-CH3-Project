#include "BTTask_FindPlayerbyActor.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include <Kismet/GameplayStatics.h>

UBTTask_FindPlayerbyActor::UBTTask_FindPlayerbyActor()
{
	NodeName = TEXT("Find Player Location");
}

EBTNodeResult::Type UBTTask_FindPlayerbyActor::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();

	//플레이어 위치찾는코드 월드에서 가져오는거라 반드시 찾기때문에 의미가 없음 테스트용이므로 나중에 삭제
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	BlackboardComp->SetValueAsVector(TEXT("PlayerVector"), PlayerPawn->GetActorLocation());
	//여기까지

	/*추후에 액터를 읽는 아래의 코드로변경해야함
	if (!BlackboardComp)
	{
		return EBTNodeResult::Failed;
	}

	AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TEXT("TargetActor")));

	if (!TargetActor)
	{
		return EBTNodeResult::Failed;
	}

	BlackboardComp->SetValueAsVector(TEXT("PlayerVector"), TargetActor->GetActorLocation());*/

	return EBTNodeResult::Succeeded;
}

