#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectPtr.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "ForgottenHUDWidget.generated.h"

class UForgottenMiniMap;
class UProgressBar;
class UTextBlock;

UCLASS()
class FORGOTTENVIGILANCE_API UForgottenHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	FORCEINLINE UForgottenMiniMap* GetMiniMapWidget() const { return MiniMap; }

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void HandleHealthChanged(float CurrentHealth, float MaxHealth);

	UFUNCTION()
	void HandleHeatChanged(float CurrentHeat, float MaxHeat);

	UFUNCTION()
	void HandleOverheatChanged(bool bIsOverHeated);
	
	UFUNCTION()
	void HandleScoreChanged(int32 NewScore);

	UFUNCTION()
	void HandleObjectiveChanged(int32 CurrentProgress, int32 RequiredProgress);

	UFUNCTION()
	void HandleRemainingTimeChanged(float RemainingTime);
	
	UFUNCTION()
	void HandleKillCountChanged(int32 NewKillCount);

	UFUNCTION()
	void HideKillMarker();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> HeatBarImage;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> HeatBarMaterial;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> OverheatWarningText;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ScoreText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ObjectiveText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TimerText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UForgottenMiniMap> MiniMap;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> KillMarkerImage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HUD")
	float KillMarkerDuration;
	
private:
	FTimerHandle KillMarkerTimerHandle;
};
