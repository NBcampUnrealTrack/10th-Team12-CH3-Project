
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VisionStealComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FORGOTTENVIGILANCE_API UVisionStealComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UFUNCTION()
	void ToggleVisionSteal();

	UVisionStealComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
