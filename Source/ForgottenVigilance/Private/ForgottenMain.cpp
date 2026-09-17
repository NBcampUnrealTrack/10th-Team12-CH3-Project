#include "ForgottenMain.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Animation/WidgetAnimation.h"

void UForgottenMain::NativeConstruct()
{
	Super::NativeConstruct();

	if (Anim_Main)
	{
		PlayAnimation(Anim_Main);
	}

	if (StartButton)
	{
		StartButton->OnHovered.AddDynamic(
			this,
		    &UForgottenMain::OnStartButtonHovered);

		StartButton->OnUnhovered.AddDynamic(
			this,
		    &UForgottenMain::OnStartButtonUnhovered);

		StartButton->OnClicked.AddDynamic(
		    this,
		    &UForgottenMain::OnStartButtonClicked);
	}

	if (LoadButton)
	{
		LoadButton->OnHovered.AddDynamic(
			this,
		    &UForgottenMain::OnLoadButtonHovered);

		LoadButton->OnUnhovered.AddDynamic(
			this,
		    &UForgottenMain::OnLoadButtonUnhovered);

		LoadButton->OnClicked.AddDynamic(
		    this,
		    &UForgottenMain::OnLoadButtonClicked);
	}

	if (ExitButton)
	{
		ExitButton->OnHovered.AddDynamic(
			this,
		    &UForgottenMain::OnExitButtonHovered);

		ExitButton->OnUnhovered.AddDynamic(
			this,
		    &UForgottenMain::OnExitButtonUnhovered);

		ExitButton->OnClicked.AddDynamic(
		    this,
		    &UForgottenMain::OnExitButtonClicked);
	}
}

void UForgottenMain::OnStartButtonHovered()
{
	Start_Red->SetVisibility(ESlateVisibility::Visible);
	Start_Cyan->SetVisibility(ESlateVisibility::Visible);

	if (Anim_StartHover)
	{
		PlayAnimation(Anim_StartHover);
	}
}

void UForgottenMain::OnStartButtonUnhovered()
{
	Start_Red->SetVisibility(ESlateVisibility::Hidden);
	Start_Cyan->SetVisibility(ESlateVisibility::Hidden);
}

void UForgottenMain::OnLoadButtonHovered()
{
	Load_Red->SetVisibility(ESlateVisibility::Visible);
	Load_Cyan->SetVisibility(ESlateVisibility::Visible);
	
	if (Anim_LoadHover)
	{
		PlayAnimation(Anim_LoadHover);
	}
}

void UForgottenMain::OnLoadButtonUnhovered()
{
	Load_Red->SetVisibility(ESlateVisibility::Hidden);
	Load_Cyan->SetVisibility(ESlateVisibility::Hidden);
}

void UForgottenMain::OnExitButtonHovered()
{
	Exit_Red->SetVisibility(ESlateVisibility::Visible);
	Exit_Cyan->SetVisibility(ESlateVisibility::Visible);

	if (Anim_ExitHover)
	{
		PlayAnimation(Anim_ExitHover);
	}
}

void UForgottenMain::OnExitButtonUnhovered()
{
	Exit_Red->SetVisibility(ESlateVisibility::Hidden);
	Exit_Cyan->SetVisibility(ESlateVisibility::Hidden);
}

void UForgottenMain::OnStartButtonClicked()
{
	UGameplayStatics::OpenLevel(
	    GetWorld(),
	    FName(TEXT("MainLevel")));
}

void UForgottenMain::OnLoadButtonClicked()
{
	UGameplayStatics::OpenLevel(
	    GetWorld(),
	    FName(TEXT("MainLevel")));
}

void UForgottenMain::OnExitButtonClicked()
{
	UKismetSystemLibrary::QuitGame(
	    GetWorld(),
	    nullptr,
	    EQuitPreference::Quit,
	    false);
}
