#include "ForgottenGameClear.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Animation/WidgetAnimation.h"

void UForgottenGameClear::NativeConstruct()
{
	Super::NativeConstruct();

	if (Anim_Glitch)
	{
		PlayAnimation(Anim_Glitch);
	}


	if (ExitButton)
	{
		ExitButton->OnHovered.AddDynamic(
			this,
			&UForgottenGameClear::OnExitButtonHovered);

		ExitButton->OnUnhovered.AddDynamic(
			this,
			&UForgottenGameClear::OnExitButtonUnhovered);
		ExitButton->OnClicked.AddDynamic(
			this,
			&UForgottenGameClear::OnExitButtonClicked);
	}
}


void UForgottenGameClear::OnExitButtonHovered()
{
	Exit_Red->SetVisibility(ESlateVisibility::Visible);
	Exit_Cyan->SetVisibility(ESlateVisibility::Visible);

	if (Anim_ExitHover)
	{
		PlayAnimation(Anim_ExitHover);
	}
}

void UForgottenGameClear::OnExitButtonUnhovered()
{
	Exit_Red->SetVisibility(ESlateVisibility::Hidden);
	Exit_Cyan->SetVisibility(ESlateVisibility::Hidden);
}

void UForgottenGameClear::OnExitButtonClicked()
{
	UGameplayStatics::OpenLevel(
		GetWorld(),
		FName(TEXT("TitleMap")));
}
