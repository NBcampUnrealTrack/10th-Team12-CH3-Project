#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_BossAttack.generated.h"

struct FBTBossAttackMemory
{
	float RemainingTime = 0.0f;
};

UCLASS()
class FORGOTTENVIGILANCE_API UBTTask_BossAttack : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_BossAttack();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual uint16 GetInstanceMemorySize() const override;
};