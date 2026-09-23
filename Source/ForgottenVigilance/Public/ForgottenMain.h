#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ForgottenMain.generated.h"

class UButton;
class UTextBlock;
class UWidgetAnimation;

UCLASS()
class FORGOTTENVIGILANCE_API UForgottenMain : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> StartButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ExitButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Start_Red;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Start_Cyan;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Exit_Red;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Exit_Cyan;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> Anim_Main;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> Anim_StartHover;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> Anim_ExitHover;


	UFUNCTION()
	void OnStartButtonHovered();

	UFUNCTION()
	void OnStartButtonUnhovered();

	UFUNCTION()
	void OnExitButtonHovered();

	UFUNCTION()
	void OnExitButtonUnhovered();


	UFUNCTION()
	void OnStartButtonClicked();

	UFUNCTION()
	void OnExitButtonClicked();


};
