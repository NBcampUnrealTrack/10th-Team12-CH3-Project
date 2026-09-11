#include "BTService_CheckAttackDistance.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "GameFramework/Pawn.h"
#include "Engine/Engine.h"

UBTService_CheckAttackDistance::UBTService_CheckAttackDistance()
{
	NodeName = TEXT("Check Attack Distance");

	Interval = 0.1f;
	RandomDeviation = 0.0f;
}

void UBTService_CheckAttackDistance::TickNode(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory,
    float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();

	if (!BlackboardComp)
	{
		return;
	}

	AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TEXT("TargetActor")));

	if (!TargetActor)
	{
		BlackboardComp->SetValueAsBool(TEXT("IsAttack"), false);

		return;
	}

	AAIController* AIcontroller = OwnerComp.GetAIOwner();

	if (!AIcontroller)
	{
		BlackboardComp->SetValueAsBool(TEXT("IsAttack"), false);
		return;
	}

	APawn* AIPawn = AIcontroller->GetPawn();

	if (!AIPawn)
	{
		BlackboardComp->SetValueAsBool(TEXT("IsAttack"), false);

		return;
	}

	float Distance = FVector::Dist(
	    AIPawn->GetActorLocation(),
	    TargetActor->GetActorLocation());

	

	bool bCanAttack = Distance <= AttackDistance;


	BlackboardComp->SetValueAsBool(TEXT("IsAttack"), bCanAttack);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(1, 0.2f, FColor::Green, FString::Printf(TEXT("AI Distance: %.1f cm"), Distance));
		GEngine->AddOnScreenDebugMessage(1, 0.2f, FColor::Green, FString::Printf(TEXT("AI bool: %d cm"), bCanAttack));
	}
}
