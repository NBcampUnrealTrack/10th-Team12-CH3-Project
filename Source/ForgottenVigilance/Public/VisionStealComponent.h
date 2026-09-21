
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VisionStealComponent.generated.h"

class AAIBaseCharacter;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class FORGOTTENVIGILANCE_API UVisionStealComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION()
	void ToggleVisionSteal();

	UVisionStealComponent();

protected:
	virtual void BeginPlay() override;

private:
	AAIBaseCharacter* FindTargetByCrosshair() const;

	UPROPERTY(EditAnywhere, Category = "VisionSteal")
	float MaxTargetDistance = 3000.0f;
};
