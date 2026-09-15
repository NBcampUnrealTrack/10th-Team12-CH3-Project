#pragma once

#include "CoreMinimal.h"
#include "AIBaseController.h"
#include "AIHearController.generated.h"

class UAISenseConfig_Hearing;

UCLASS()
class FORGOTTENVIGILANCE_API AAIHearController : public AAIBaseController
{
	GENERATED_BODY()
	
public:
	AAIHearController();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UAISenseConfig_Hearing> HearingConfig;
};
