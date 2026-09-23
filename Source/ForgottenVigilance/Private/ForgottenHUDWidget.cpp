#include "ForgottenHUDWidget.h"
#include "HealthComponent.h"
#include "MainWeaponComponent.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "ForgottenGameState.h"
#include "TimerManager.h"
#include "OptimusPrimeCharacter.h"
#include "Animation/WidgetAnimation.h"
#include "VisionStealComponent.h"

namespace
{
constexpr float DefaultKillMarkerDuration = 0.5f;
constexpr float DefaultHitMarkerDuration = 0.15f;
constexpr float BossHealthZeroThreshold = 0.0f;
constexpr float DefaultDamageOverlayDuration = 0.35f;
}

void UForgottenHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	HandleBossActiveChanged(false);
	HandleVisionStealStateChanged(false);

	DamageOverlayDuration = DefaultDamageOverlayDuration;

	if (DamageNoiseOverlay)
	{
		DamageNoiseOverlay->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (HeatBarImage)
	{
		HeatBarMaterial = HeatBarImage->GetDynamicMaterial();
	}

	HitMarkerDuration = DefaultHitMarkerDuration;

	if (HitMarkerImage)
	{
		HitMarkerImage->SetVisibility(ESlateVisibility::Collapsed);
	}

	APawn* OwningPawn = GetOwningPlayerPawn();
	if (!OwningPawn)
	{
		return;
	}

	if (AOptimusPrimeCharacter* PlayerCharacter = Cast<AOptimusPrimeCharacter>(OwningPawn))
	{
		PlayerCharacter->OnPlayerDamaged.AddDynamic(this, &UForgottenHUDWidget::HandlePlayerDamaged);
	}

	if (UVisionStealComponent* VisionSteal = OwningPawn->FindComponentByClass<UVisionStealComponent>())
	{
		VisionSteal->OnVisionStealChargeChanged.AddDynamic(this, &UForgottenHUDWidget::HandleVisionStealChargeChanged);
		VisionSteal->OnVisionStealStateChanged.AddDynamic(this, &UForgottenHUDWidget::HandleVisionStealStateChanged);

		HandleVisionStealChargeChanged(VisionSteal->GetCurrentCharges(), VisionSteal->GetMaxCharges());
	}

	if (UHealthComponent* HealthComp = OwningPawn->FindComponentByClass<UHealthComponent>())
	{
		HealthComp->OnHealthChanged.AddDynamic(
			this,
			&UForgottenHUDWidget::HandleHealthChanged);

		HandleHealthChanged(
			HealthComp->GetCurrentHealth(),
			HealthComp->GetMaxHealth());
	}

	if (UMainWeaponComponent* WeaponComp = OwningPawn->FindComponentByClass<UMainWeaponComponent>())
	{
		WeaponComp->OnHeatChanged.AddDynamic(
			this,
			&UForgottenHUDWidget::HandleHeatChanged);

		WeaponComp->OnOverheatStateChanged.AddDynamic(
			this,
			&UForgottenHUDWidget::HandleOverheatChanged);

		if (HeatBarMaterial)
		{
			HeatBarMaterial->SetScalarParameterValue(
				FName("Percent"),
				WeaponComp->GetHeatRatio());
		}

		HandleOverheatChanged(WeaponComp->IsOverheated());
		WeaponComp->OnShotHit.AddDynamic(this, &UForgottenHUDWidget::HandleShotHit);
	}

	AForgottenGameState* ForgottenGameState = GetWorld()->GetGameState<AForgottenGameState>();

	if (!ForgottenGameState)
	{
		return;
	}

	ForgottenGameState->OnScoreChanged.AddDynamic(this, &UForgottenHUDWidget::HandleScoreChanged);
	ForgottenGameState->OnKillCountChanged.AddDynamic(this, &UForgottenHUDWidget::HandleKillCountChanged);
	ForgottenGameState->OnObjectiveProgressChanged.AddDynamic(this, &UForgottenHUDWidget::HandleObjectiveChanged);
	ForgottenGameState->OnRemainingTimeChanged.AddDynamic(this, &UForgottenHUDWidget::HandleRemainingTimeChanged);
	ForgottenGameState->OnBossHealthUpdated.AddDynamic(this, &UForgottenHUDWidget::HandleBossHealthUpdated);
	ForgottenGameState->OnBossActiveChanged.AddDynamic(this, &UForgottenHUDWidget::HandleBossActiveChanged);

	HandleScoreChanged(ForgottenGameState->GetScore());
	HandleObjectiveChanged(
		ForgottenGameState->GetObjectiveProgress(),
		ForgottenGameState->GetRequiredObjectiveProgress());

	KillMarkerDuration = DefaultKillMarkerDuration;

	if (KillMarkerImage)
	{
		KillMarkerImage->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UForgottenHUDWidget::HandleHealthChanged(float CurrentHealth, float MaxHealth)
{
	HealthBar->SetPercent(CurrentHealth / MaxHealth);
}

void UForgottenHUDWidget::HandleKillCountChanged(int32 NewKillCount)
{
	if (!KillMarkerImage)
	{
		return;
	}

	HideHitMarker();
	KillMarkerImage->SetVisibility(ESlateVisibility::HitTestInvisible);

	UWorld* World = GetWorld();

	if (!World)
	{
		return;
	}

	World->GetTimerManager().SetTimer(
		KillMarkerTimerHandle,
		this,
		&UForgottenHUDWidget::HideKillMarker,
		KillMarkerDuration,
		false);
}

void UForgottenHUDWidget::HideKillMarker()
{
	if (!KillMarkerImage)
	{
		return;
	}

	KillMarkerImage->SetVisibility(ESlateVisibility::Collapsed);
}

void UForgottenHUDWidget::HandleHeatChanged(
	float CurrentHeat,
	float MaxHeat)
{
	if (HeatBarMaterial && MaxHeat > 0.0f)
	{
		const float HeatPercent = CurrentHeat / MaxHeat;

		HeatBarMaterial->SetScalarParameterValue(FName("Percent"), HeatPercent);
	}
}

void UForgottenHUDWidget::HandleOverheatChanged(bool bIsOverheated)
{
	if (HeatBarImage)
	{
		if (bIsOverheated)
		{
			HeatBarImage->SetColorAndOpacity(FLinearColor::Red);

			OverheatWarningText->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			HeatBarImage->SetColorAndOpacity(FLinearColor::White);

			OverheatWarningText->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	if (Anim_Warning)
	{
		if (bIsOverheated)
		{
			PlayAnimation(Anim_Warning);
		}
	}
}

void UForgottenHUDWidget::HandleScoreChanged(int32 NewScore)
{
	ScoreText->SetText(FText::AsNumber(NewScore));
}

void UForgottenHUDWidget::HandleObjectiveChanged(int32 CurrentProgress, int32 RequiredProgress)
{
	ObjectiveText->SetText(FText::FromString(
		FString::Printf(TEXT("%d / %d"), CurrentProgress, RequiredProgress)));
}

void UForgottenHUDWidget::HandleRemainingTimeChanged(float RemainingTime)
{
	const int32 TotalSeconds = FMath::FloorToInt(RemainingTime);
	const int32 Minutes = TotalSeconds / 60;
	const int32 Seconds = TotalSeconds % 60;

	TimerText->SetText(FText::FromString(
		FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds)));
}

void UForgottenHUDWidget::HandleShotHit(bool bHitCharacter)
{
	if (!bHitCharacter || !HitMarkerImage)
	{
		return;
	}

	HitMarkerImage->SetVisibility(ESlateVisibility::HitTestInvisible);

	UWorld* World = GetWorld();

	if (!World)
	{
		return;
	}

	World->GetTimerManager().SetTimer(
		HitMarkerTimerHandle,
		this,
		&UForgottenHUDWidget::HideHitMarker,
		HitMarkerDuration,
		false);
}

void UForgottenHUDWidget::HideHitMarker()
{
	if (!HitMarkerImage)
	{
		return;
	}

	HitMarkerImage->SetVisibility(ESlateVisibility::Collapsed);
}

void UForgottenHUDWidget::HandleBossHealthUpdated(float CurrentHealth, float MaxHealth)
{
	if (!BossHealthBar || MaxHealth <= BossHealthZeroThreshold)
	{
		return;
	}

	BossHealthBar->SetPercent(CurrentHealth / MaxHealth);
}

void UForgottenHUDWidget::HandleBossActiveChanged(bool bIsActive)
{
	const ESlateVisibility BossVisibility = bIsActive
		                                        ? ESlateVisibility::HitTestInvisible
		                                        : ESlateVisibility::Collapsed;

	PlayAnimation(Anim_BossHP);

	if (BossHealthBar)
	{
		BossHealthBar->SetVisibility(BossVisibility);
	}

	if (BossHealthFrameImage)
	{
		BossHealthFrameImage->SetVisibility(BossVisibility);
	}

	if (BossNameText)
	{
		BossNameText->SetVisibility(BossVisibility);
	}
}

void UForgottenHUDWidget::HandlePlayerDamaged()
{
	if (!DamageNoiseOverlay)
	{
		return;
	}

	DamageNoiseOverlay->SetVisibility(ESlateVisibility::HitTestInvisible);

	UWorld* World = GetWorld();

	if (!World)
	{
		return;
	}

	World->GetTimerManager().SetTimer(
		DamageOverlayTimerHandle,
		this,
		&UForgottenHUDWidget::HideDamageOverlay,
		DamageOverlayDuration,
		false);
}

void UForgottenHUDWidget::HideDamageOverlay()
{
	if (!DamageNoiseOverlay)
	{
		return;
	}

	DamageNoiseOverlay->SetVisibility(ESlateVisibility::Collapsed);
}

void UForgottenHUDWidget::HandleVisionStealChargeChanged(int32 CurrentCharges, int32 MaxCharges)
{
	if (!VisionStealChargeText)
	{
		return;
	}

	VisionStealChargeText->SetText(FText::FromString(
		FString::Printf(TEXT("%d / %d"), CurrentCharges, MaxCharges)));
}

void UForgottenHUDWidget::HandleVisionStealStateChanged(bool bIsActive)
{
	if (!VisionStealActiveOverlay)
	{
		return;
	}

	VisionStealActiveOverlay->SetVisibility(bIsActive
		                                        ? ESlateVisibility::HitTestInvisible
		                                        : ESlateVisibility::Collapsed);
}
