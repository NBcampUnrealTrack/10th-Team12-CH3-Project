#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ForgottenMain.generated.h"

class UButton;

UCLASS()
class FORGOTTENVIGILANCE_API UForgottenMain : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> StartButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> LoadButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ExitButton;

	UFUNCTION()
	void OnStartButtonClicked();

	UFUNCTION()
	void OnLoadButtonClicked();

	UFUNCTION()
	void OnExitButtonClicked();

	
};
