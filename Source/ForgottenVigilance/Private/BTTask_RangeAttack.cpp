#include "BTTask_RangeAttack.h"

#include "BehaviorTree/BehaviorTreeComponent.h"
#include "AIController.h"
#include "AIRangeCharacter.h"
#include "AIRangeWeaponComponent.h"

UBTTask_RangeAttack::UBTTask_RangeAttack()
{
	NodeName = TEXT("Ranged Attack");
}

EBTNodeResult::Type UBTTask_RangeAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();

	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	AAIRangeCharacter* AICharacter =
	    Cast<AAIRangeCharacter>(AIController->GetPawn());

	if (!AICharacter)
	{
		return EBTNodeResult::Failed;
	}

	UAIRangeWeaponComponent* Weapon =
	    AICharacter->FindComponentByClass<UAIRangeWeaponComponent>();

	if (!Weapon)
	{
		return EBTNodeResult::Failed;
	}

	Weapon->FireGun();

	return EBTNodeResult::Succeeded;
}


