#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "OptimusPrimePlayerController.generated.h"

class UInputAction;
class UInputMappingContext;
class UUserWidget;

class UForgottenMain;
class UForgottenHUDWidget;
class UForgottenGameOver;

UCLASS()
class FORGOTTENVIGILANCE_API AOptimusPrimePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AOptimusPrimePlayerController();
	float GetAimRotationSpeed() const;

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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<UInputAction> ShootAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
	TSubclassOf<UForgottenHUDWidget> HUDWidgetClass;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD")
	TObjectPtr<UForgottenHUDWidget> HUDWidgetInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu")
	TSubclassOf<UForgottenMain> MainMenuWidgetClass;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Menu")
	TObjectPtr<UForgottenMain> MainMenuWidgetInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Over")
	TSubclassOf<UForgottenGameOver> GameOverWidgetClass;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Over")
	TObjectPtr<UForgottenGameOver> GameOverWidgetInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level")
	FName MainMenuLevelName; 

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowMainMenu();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowHUD();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowGameOver();

	UFUNCTION(BlueprintCallable, Category = "Game")
	void StartGame();

	UFUNCTION(BlueprintCallable, Category = "Game")
	void RetryGame();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	float InitialCameraPitch;

	UPROPERTY(EditDefaultsOnly, Category = "Camera", meta = (ClampMin = "-89.0", ClampMax = "89.0"))
	float CameraPitchMin = -70.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Camera", meta = (ClampMin = "-89.0", ClampMax = "89.0"))
	float CameraPitchMax = 35.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Aiming")
	float AimRotationSpeed;

	void ClearAllWidgets();
};
