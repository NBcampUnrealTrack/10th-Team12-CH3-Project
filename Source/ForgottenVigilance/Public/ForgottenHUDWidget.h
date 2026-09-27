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
class UWidgetAnimation;

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

	UFUNCTION()
	void HandleShotHit(bool bHitCharacter);

	UFUNCTION()
	void HideHitMarker();

	UFUNCTION()
	void HandleBossHealthUpdated(float CurrentHealth, float MaxHealth);

	UFUNCTION()
	void HandleBossActiveChanged(bool bIsActive);

	UFUNCTION()
	void HandlePlayerDamaged();

	UFUNCTION()
	void HideDamageOverlay();

	UFUNCTION()
	void HandleVisionStealChargeChanged(int32 CurrentCharges, int32 MaxCharges);

	UFUNCTION()
	void HandleVisionStealStateChanged(bool bIsActive);
	
	UFUNCTION()
	void HandleDashCooldownChanged(float RemainingRatio);
	
	UFUNCTION()
	void HandleBossStaggerUpdated(float StaggerRatio);

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

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> HitMarkerImage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HUD")
	float HitMarkerDuration;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> BossHealthBar;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> BossNameText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> BossHealthFrameImage;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> Anim_Warning;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> Anim_BossHP;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> DamageNoiseOverlay;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HUD")
	float DamageOverlayDuration;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> VisionStealChargeText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> VisionStealActiveOverlay;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> DashIconImage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HUD")
	float DashIconMinAlpha;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> BossStaggerBar;

private:
	FTimerHandle KillMarkerTimerHandle;
	FTimerHandle HitMarkerTimerHandle;
	FTimerHandle DamageOverlayTimerHandle;
};
