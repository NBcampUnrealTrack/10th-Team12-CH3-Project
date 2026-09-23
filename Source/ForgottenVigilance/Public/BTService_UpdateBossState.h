#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_UpdateBossState.generated.h"

UCLASS()
class FORGOTTENVIGILANCE_API UBTService_UpdateBossState : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_UpdateBossState();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
