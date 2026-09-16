#include "ForgottenGameState.h"

#include "ForgottenHUDWidget.h"
#include "ForgottenMain.h"
#include "OptimusPrimePlayerController.h"

AForgottenGameState::AForgottenGameState()
{
}

void AForgottenGameState::BeginPlay()
{
	Super::BeginPlay();
	
	PlayerController = GetWorld()->GetFirstPlayerController<AOptimusPrimePlayerController>();
	
	if (!PlayerController)
	{
		return;
	}

	FString PureLevelName = GetWorld()->GetOuter()->GetName();
	
	if (PureLevelName.Contains(TEXT("TitleMap")))
	{
		PlayerController->ShowMainMenu();
	}
	else if (PureLevelName.Contains(TEXT("MainLevel")))
	{
		PlayerController->ShowHUD();
	}
}
