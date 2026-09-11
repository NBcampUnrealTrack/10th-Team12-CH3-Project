#include "OptimusPrimePlayerController.h"
#include "Blueprint/UserWidget.h"
#include "EnhancedInputSubsystems.h"

namespace
{
constexpr float DefaultInitialCameraPitch = -30.0f;
constexpr float DefaultAimRotationSpeed = 540.0f;
}

AOptimusPrimePlayerController::AOptimusPrimePlayerController()
    : InputMappingContext(nullptr)
    , MoveAction(nullptr)
    , LookAction(nullptr)
    , JumpAction(nullptr)
    , SprintAction(nullptr)
    , ShootAction(nullptr)
    , CameraRotateAction(nullptr)
    , HUDWidgetClass(nullptr)
    , InitialCameraPitch(DefaultInitialCameraPitch)
    , AimRotationSpeed(DefaultAimRotationSpeed)
{
}

void AOptimusPrimePlayerController::BeginPlay()
{
	Super::BeginPlay();

	SetControlRotation(FRotator(InitialCameraPitch, GetControlRotation().Yaw, 0.0f));

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

	if (HUDWidgetClass)
	{
		HUDWidgetInstance = CreateWidget<UUserWidget>(this, HUDWidgetClass);
		if (HUDWidgetInstance)
		{
			HUDWidgetInstance->AddToViewport();
		}
	}
}

UUserWidget* AOptimusPrimePlayerController::GetHUDWidget() const
{
	return HUDWidgetInstance;
}

void AOptimusPrimePlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
