#include "ForgottenGameOver.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

void UForgottenGameOver::NativeConstruct()
{
	Super::NativeConstruct();

	if (Anim_Glitch)
	{
		PlayAnimation(Anim_Glitch);
	}

	if (RetryButton)
	{
		RetryButton->OnClicked.AddDynamic(
		    this,
		    &UForgottenGameOver::OnRetryButtonClicked);
	}

	if (ExitButton)
	{
		ExitButton->OnClicked.AddDynamic(
		    this,
		    &UForgottenGameOver::OnExitButtonClicked);
	}
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
