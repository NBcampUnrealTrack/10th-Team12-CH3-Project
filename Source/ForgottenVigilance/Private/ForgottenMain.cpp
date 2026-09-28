#include "ForgottenMain.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Animation/WidgetAnimation.h"
#include "Blueprint/UserWidget.h"

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

	if (ControlsButton)
	{
		ControlsButton->OnHovered.AddDynamic(
		    this,
		    &UForgottenMain::OnControlsButtonHovered);

		ControlsButton->OnUnhovered.AddDynamic(
		    this,
		    &UForgottenMain::OnControlsButtonUnhovered);

		ControlsButton->OnClicked.AddDynamic(
		    this,
		    &UForgottenMain::OnControlsButtonClicked);
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

void UForgottenMain::OnControlsButtonHovered()
{
	Controls_Red->SetVisibility(ESlateVisibility::Visible);
	Controls_Cyan->SetVisibility(ESlateVisibility::Visible);

	if (Anim_ControlsHover)
	{
		PlayAnimation(Anim_ControlsHover);
	}
}

void UForgottenMain::OnControlsButtonUnhovered()
{
	Controls_Red->SetVisibility(ESlateVisibility::Hidden);
	Controls_Cyan->SetVisibility(ESlateVisibility::Hidden);
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

void UForgottenMain::OnControlsButtonClicked()
{
	if (ControlGuideWidget && ControlGuideWidget->IsInViewport())
	{
		return;
	}

	static const FSoftClassPath ControlGuidePath(TEXT("/Game/UI/WBP_ForgottenControls.WBP_ForgottenControls_C"));

	UClass* ControlGuideClass = ControlGuidePath.TryLoadClass<UUserWidget>();

	if (!ControlGuideClass)
	{
		return;
	}

	ControlGuideWidget = CreateWidget<UUserWidget>(GetOwningPlayer(), ControlGuideClass);

	if (ControlGuideWidget)
	{
		ControlGuideWidget->AddToViewport();
		FInputModeUIOnly InputMode;

		InputMode.SetWidgetToFocus(ControlGuideWidget->TakeWidget());

		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

		APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();

		if (PlayerController)
		{
			PlayerController->SetInputMode(InputMode);
			PlayerController->bShowMouseCursor = true;
		}
	}
}



void UForgottenMain::OnExitButtonClicked()
{
	UKismetSystemLibrary::QuitGame(
		GetWorld(),
		nullptr,
		EQuitPreference::Quit,
		false);
}
