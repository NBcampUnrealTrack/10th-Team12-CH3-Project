#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "OptimusPrimePlayerController.generated.h"

class UInputMappingContext; // IMC 관련 전방 선언
class UInputAction;         // IA 관련 전방 선언

UCLASS()
class FORGOTTENVIGILANCE_API AOptimusPrimePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AOptimusPrimePlayerController();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<UInputMappingContext> InputMappingContext;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<UInputAction> LookAction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<UInputAction> SprintAction;

	virtual void BeginPlay() override;
};
