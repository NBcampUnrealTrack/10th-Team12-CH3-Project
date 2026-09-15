#include "ForgottenMain.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void UForgottenMain::NativeConstruct()
{
	Super::NativeConstruct();

	if (StartButton)
	{
		StartButton->OnClicked.AddDynamic(
		    this,
		    &UForgottenMain::OnStartButtonClicked);
	}

	if (LoadButton)
	{
		LoadButton->OnClicked.AddDynamic(
		    this,
		    &UForgottenMain::OnLoadButtonClicked);
	}

	if (ExitButton)
	{
		ExitButton->OnClicked.AddDynamic(
		    this,
		    &UForgottenMain::OnExitButtonClicked);
	}
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
