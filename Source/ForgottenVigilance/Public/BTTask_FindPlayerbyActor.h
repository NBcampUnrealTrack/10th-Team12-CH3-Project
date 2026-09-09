#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_FindPlayerbyActor.generated.h"


UCLASS()
class FORGOTTENVIGILANCE_API UBTTask_FindPlayerbyActor : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_FindPlayerbyActor();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
