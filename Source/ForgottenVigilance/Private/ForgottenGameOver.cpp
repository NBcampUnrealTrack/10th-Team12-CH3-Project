#include "ForgottenGameOver.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Animation/WidgetAnimation.h"

void UForgottenGameOver::NativeConstruct()
{
	Super::NativeConstruct();

	if (Anim_Glitch)
	{
		PlayAnimation(Anim_Glitch);
	}

	if (RetryButton)
	{
		RetryButton->OnHovered.AddDynamic(
			this,
			&UForgottenGameOver::OnRetryButtonHovered);

		RetryButton->OnUnhovered.AddDynamic(
		    this,
		    &UForgottenGameOver::OnRetryButtonUnhovered);

		RetryButton->OnClicked.AddDynamic(
		    this,
		    &UForgottenGameOver::OnRetryButtonClicked);
	}

	if (ExitButton)
	{
		ExitButton->OnHovered.AddDynamic(
			this,
		    &UForgottenGameOver::OnExitButtonHovered);

		ExitButton->OnUnhovered.AddDynamic(
			this,
		    &UForgottenGameOver::OnExitButtonUnhovered);

		ExitButton->OnClicked.AddDynamic(
		    this,
		    &UForgottenGameOver::OnExitButtonClicked);
	}
}

void UForgottenGameOver::OnRetryButtonHovered()
{
	Retry_Red->SetVisibility(ESlateVisibility::Visible);
	Retry_Cyan->SetVisibility(ESlateVisibility::Visible);

	if (Anim_RetryHover)
	{
		PlayAnimation(Anim_RetryHover);
	}
}

void UForgottenGameOver::OnRetryButtonUnhovered()
{
	Retry_Red->SetVisibility(ESlateVisibility::Hidden);
	Retry_Cyan->SetVisibility(ESlateVisibility::Hidden);
}

void UForgottenGameOver::OnExitButtonHovered()
{
	Exit_Red->SetVisibility(ESlateVisibility::Visible);
	Exit_Cyan->SetVisibility(ESlateVisibility::Visible);

	if (Anim_ExitHover)
	{
		PlayAnimation(Anim_ExitHover);
	}
}

void UForgottenGameOver::OnExitButtonUnhovered()
{
	Exit_Red->SetVisibility(ESlateVisibility::Hidden);
	Exit_Cyan->SetVisibility(ESlateVisibility::Hidden);
}

void UForgottenGameOver::OnRetryButtonClicked()
{
	UGameplayStatics::OpenLevel(
	    GetWorld(),
	    FName(TEXT("MainLevel")));
}

void UForgottenGameOver::OnExitButtonClicked()
{
	UGameplayStatics::OpenLevel(
	    GetWorld(),
	    FName(TEXT("TitleMap")));
}
