#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ForgottenGameClear.generated.h"

class UButton;
class UTextBlock;
class UWidgetAnimation;

UCLASS()
class FORGOTTENVIGILANCE_API UForgottenGameClear : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ExitButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Exit_Red;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Exit_Cyan;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> Anim_Glitch;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> Anim_ExitHover;


	UFUNCTION()
	void OnExitButtonHovered();

	UFUNCTION()
	void OnExitButtonUnhovered();

	UFUNCTION()
	void OnExitButtonClicked();
};
