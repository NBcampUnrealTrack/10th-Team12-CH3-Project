#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SubWeaponComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FORGOTTENVIGILANCE_API USubWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USubWeaponComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

};
