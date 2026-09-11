#include "ForgottenHUDWidget.h"
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
