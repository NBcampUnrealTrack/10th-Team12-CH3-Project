#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ForgottenMinimapCaptureActor.generated.h"

UCLASS()
class FORGOTTENVIGILANCE_API AForgottenMinimapCaptureActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AForgottenMinimapCaptureActor();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

};
