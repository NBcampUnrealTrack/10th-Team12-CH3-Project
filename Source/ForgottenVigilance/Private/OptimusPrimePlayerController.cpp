
#include "OptimusPrimePlayerController.h"
#include "EnhancedInputSubsystems.h"


AOptimusPrimePlayerController::AOptimusPrimePlayerController()
    : InputMappingContext(nullptr)
    , MoveAction(nullptr)
    , JumpAction(nullptr)
    , LookAction(nullptr)
    , SprintAction(nullptr)
{
}

void AOptimusPrimePlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer)
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!Subsystem)
	{
		return;
	}

	if (InputMappingContext)
	{
		Subsystem->AddMappingContext(InputMappingContext, 0);
	}
}
