#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "ForgottenGameState.generated.h"

class AOptimusPrimePlayerController;
class UForgottenGameOver;
class UForgottenMain;
class UForgottenHUDWidget;

UCLASS()
class FORGOTTENVIGILANCE_API AForgottenGameState : public AGameState
{
	GENERATED_BODY()
	
protected:
	AForgottenGameState();
	virtual void BeginPlay() override;
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player", meta = (AllowPrivateAccess = true))
	TObjectPtr<AOptimusPrimePlayerController> PlayerController;
};
