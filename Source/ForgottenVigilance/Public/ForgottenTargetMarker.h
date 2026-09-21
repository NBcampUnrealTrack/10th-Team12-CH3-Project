#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ForgottenTargetMarker.generated.h"

class AActor;

UCLASS()
class FORGOTTENVIGILANCE_API UForgottenTargetMarker : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetTarget(AActor* InTarget);

	AActor* GetTarget() const {return Target.Get();}

private:
	TWeakObjectPtr<AActor> Target;
};
