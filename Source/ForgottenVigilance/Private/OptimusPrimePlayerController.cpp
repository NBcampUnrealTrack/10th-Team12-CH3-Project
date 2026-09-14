#include "OptimusPrimePlayerController.h"
#include "Blueprint/UserWidget.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"

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

	const float MinPitch = FMath::Min(CameraPitchMin, CameraPitchMax);
	const float MaxPitch = FMath::Max(CameraPitchMin, CameraPitchMax);
	if (PlayerCameraManager)
	{
		PlayerCameraManager->ViewPitchMin = MinPitch;
		PlayerCameraManager->ViewPitchMax = MaxPitch;
	}
	SetControlRotation(FRotator(FMath::Clamp(InitialCameraPitch, MinPitch, MaxPitch),
		GetControlRotation().Yaw, 0.0f));

	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer)
	{
		return;
	}
	SetShowMouseCursor(false);
	FInputModeGameOnly InputMode;
	InputMode.SetConsumeCaptureMouseDown(false);
	SetInputMode(InputMode);
	if (UGameViewportClient* ViewportClient = LocalPlayer->ViewportClient)
	{
		ViewportClient->SetMouseCaptureMode(EMouseCaptureMode::CapturePermanently_IncludingInitialMouseDown);
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

float AOptimusPrimePlayerController::GetAimRotationSpeed() const
{
	return AimRotationSpeed;
}

void AOptimusPrimePlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
