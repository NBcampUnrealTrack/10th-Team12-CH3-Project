#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_RangeAttack.generated.h"

/**
 * 
 */
UCLASS()
class FORGOTTENVIGILANCE_API UBTTask_RangeAttack : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_RangeAttack();

protected:
	virtual EBTNodeResult::Type ExecuteTask(
	    UBehaviorTreeComponent& OwnerComp,
	    uint8* NodeMemory) override;


	
};
