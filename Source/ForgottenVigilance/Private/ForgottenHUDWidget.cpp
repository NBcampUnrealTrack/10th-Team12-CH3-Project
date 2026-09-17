#include "ForgottenHUDWidget.h"
#include "HealthComponent.h"
#include "MainWeaponComponent.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UForgottenHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (HeatBarImage)
	{
		HeatBarMaterial = HeatBarImage->GetDynamicMaterial();
	}

	APawn* OwningPawn = GetOwningPlayerPawn();
	if (!OwningPawn)
	{
		return;
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
	}
}

void UForgottenHUDWidget::HandleHealthChanged(float CurrentHealth, float MaxHealth)
{
	HealthBar->SetPercent(CurrentHealth / MaxHealth);
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
}
