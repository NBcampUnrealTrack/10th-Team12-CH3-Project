#include "ForgottenHUDWidget.h"
#include "OptimusPrimeCharacter.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "GameFramework/PlayerController.h"
#include "HealthComponent.h"
#include "MainWeaponComponent.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UForgottenHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	APawn* OwningPawn = GetOwningPlayerPawn();
	if (!OwningPawn)
	{
		return;
	}

	if (UHealthComponent* HealthComp = OwningPawn->FindComponentByClass<UHealthComponent>())
	{
		HealthComp->OnHealthChanged.AddDynamic(this, &UForgottenHUDWidget::HandleHealthChanged);
		HandleHealthChanged(HealthComp->GetCurrentHealth(), HealthComp->GetMaxHealth());
	}

	if (UMainWeaponComponent* WeaponComp = OwningPawn->FindComponentByClass<UMainWeaponComponent>())
	{
		WeaponComp->OnHeatChanged.AddDynamic(this, &UForgottenHUDWidget::HandleHeatChanged);
		WeaponComp->OnOverheatStateChanged.AddDynamic(this, &UForgottenHUDWidget::HandleOverheatChanged);

		HeatBar->SetPercent(WeaponComp->GetHeatRatio());
		HandleOverheatChanged(WeaponComp->IsOverheated());
	}
}

int32 UForgottenHUDWidget::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
	int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	const int32 BaseLayer = Super::NativePaint(Args, AllottedGeometry, MyCullingRect,
		OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
	if (!bShowDeadZoneGuides)
	{
		return BaseLayer;
	}

	const float ViewportScale = UWidgetLayoutLibrary::GetViewportScale(this);
	if (ViewportScale <= 0.0f)
	{
		return BaseLayer;
	}

	const AOptimusPrimeCharacter* Character = Cast<AOptimusPrimeCharacter>(GetOwningPlayerPawn());
	APlayerController* PlayerController = GetOwningPlayer();
	FVector LeftBoundary;
	FVector RightBoundary;
	if (!PlayerController || !Character || !Character->GetCameraDeadZoneBounds(LeftBoundary, RightBoundary))
	{
		return BaseLayer;
	}
	FVector2D LeftPosition;
	FVector2D RightPosition;
	if (!UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PlayerController,
		LeftBoundary, LeftPosition, true)
		|| !UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PlayerController,
		RightBoundary, RightPosition, true))
	{
		return BaseLayer;
	}
	// 월드 경계를 현재 화면으로 투영하며, 창 크기가 바뀌어도 월드 폭은 유지합니다.
	const FVector2D WidgetSize = AllottedGeometry.GetLocalSize();
	const float LeftX = LeftPosition.X;
	const float RightX = RightPosition.X;
	const float Thickness = DeadZoneGuideThicknessPixels / ViewportScale;
	FPaintContext PaintContext(AllottedGeometry, MyCullingRect, OutDrawElements,
		BaseLayer + 1, InWidgetStyle, bParentEnabled);
	UWidgetBlueprintLibrary::DrawLine(PaintContext, FVector2D(LeftX, 0.0f),
		FVector2D(LeftX, WidgetSize.Y), DeadZoneGuideColor, true, Thickness);
	UWidgetBlueprintLibrary::DrawLine(PaintContext, FVector2D(RightX, 0.0f),
		FVector2D(RightX, WidgetSize.Y), DeadZoneGuideColor, true, Thickness);
	return PaintContext.MaxLayer;
}

void UForgottenHUDWidget::HandleHealthChanged(float CurrentHealth, float MaxHealth)
{
	HealthBar->SetPercent(CurrentHealth / MaxHealth);
}

void UForgottenHUDWidget::HandleHeatChanged(float CurrentHeat, float MaxHeat)
{
	HeatBar->SetPercent(CurrentHeat / MaxHeat);
}

void UForgottenHUDWidget::HandleOverheatChanged(bool bIsOverheated)
{
	if (bIsOverheated)
	{
		HeatBar->SetFillColorAndOpacity(FLinearColor::Red);
		OverheatWarningText->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		HeatBar->SetFillColorAndOpacity(FLinearColor::White);
		OverheatWarningText->SetVisibility(ESlateVisibility::Collapsed);
	}
}
