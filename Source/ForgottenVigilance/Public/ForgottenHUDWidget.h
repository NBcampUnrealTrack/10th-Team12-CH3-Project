#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectPtr.h"
#include "Blueprint/UserWidget.h"
#include "ForgottenHUDWidget.generated.h"

class UProgressBar;
class UTextBlock;

UCLASS()
class FORGOTTENVIGILANCE_API UForgottenHUDWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
		int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	UFUNCTION()
	void HandleHealthChanged(float CurrentHealth, float MaxHealth);

	UFUNCTION()
	void HandleHeatChanged(float CurrentHeat, float MaxHeat);

	UFUNCTION()
	void HandleOverheatChanged(bool bIsOverHeated);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HeatBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> OverheatWarningText;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Camera|DeadZone")
	bool bShowDeadZoneGuides = true;

	UPROPERTY(EditDefaultsOnly, Category = "Camera|DeadZone")
	FLinearColor DeadZoneGuideColor = FLinearColor(1.0f, 0.0f, 0.0f, 0.4f);

	UPROPERTY(EditDefaultsOnly, Category = "Camera|DeadZone", meta = (ClampMin = "1.0"))
	float DeadZoneGuideThicknessPixels = 2.0f;

};
