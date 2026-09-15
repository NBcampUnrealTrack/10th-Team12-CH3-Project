#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ForgottenGameOver.generated.h"

class UButton;

UCLASS()
class FORGOTTENVIGILANCE_API UForgottenGameOver : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> RetryButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ExitButton;

	UFUNCTION()
	void OnRetryButtonClicked();

	UFUNCTION()
	void OnExitButtonClicked();
};