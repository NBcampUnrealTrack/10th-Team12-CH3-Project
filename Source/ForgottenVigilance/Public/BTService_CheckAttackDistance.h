#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BTService_CheckAttackDistance.generated.h"

UCLASS()
class FORGOTTENVIGILANCE_API UBTService_CheckAttackDistance : public UBTService_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTService_CheckAttackDistance();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, Category = "AI")
	float AttackDistance = 200.0f;

};
