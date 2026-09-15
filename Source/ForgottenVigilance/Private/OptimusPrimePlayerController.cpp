#include "OptimusPrimePlayerController.h"
#include "Blueprint/UserWidget.h"
#include "ForgottenMain.h"
#include "ForgottenHUDWidget.h"
#include "ForgottenGameOver.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"
#include "Kismet/GameplayStatics.h"

namespace
{
constexpr float DefaultInitialCameraPitch = -30.0f;
constexpr float DefaultAimRotationSpeed = 540.0f;
} // namespace

AOptimusPrimePlayerController::AOptimusPrimePlayerController()
    : InputMappingContext(nullptr)
    , MoveAction(nullptr)
    , LookAction(nullptr)
    , JumpAction(nullptr)
    , SprintAction(nullptr)
    , ShootAction(nullptr)
    , HUDWidgetClass(nullptr)
    , HUDWidgetInstance(nullptr)
    , MainMenuWidgetClass(nullptr)
    , MainMenuWidgetInstance(nullptr)
    , GameOverWidgetClass(nullptr)
    , GameOverWidgetInstance(nullptr)
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

	    const FString CurrentMapName = GetWorld()->GetMapName();

	if (CurrentMapName.Contains(TEXT("TitleMap")))
	{
		ShowMainMenu();
	}
}

void AOptimusPrimePlayerController::ShowMainMenu()
{
	if (!MainMenuWidgetClass)
	{
		return;
	}

	UUserWidget* MainMenuWidget =
	    CreateWidget<UUserWidget>(this, MainMenuWidgetClass);

	if (MainMenuWidget)
	{
		MainMenuWidget->AddToViewport();
	}

	SetShowMouseCursor(true);

	FInputModeUIOnly InputMode;
	SetInputMode(InputMode);
}

void AOptimusPrimePlayerController::ShowHUD()
{
	if (!HUDWidgetClass)
	{
		return;
	}

	UUserWidget* HUDWidget =
	    CreateWidget<UUserWidget>(this, HUDWidgetClass);

	if (HUDWidget)
	{
		HUDWidget->AddToViewport();
	}

	SetShowMouseCursor(false);

	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
}

void AOptimusPrimePlayerController::ShowGameOver()
{
	if (!GameOverWidgetClass)
	{
		return;
	}

	UUserWidget* GameOverWidget =
	    CreateWidget<UUserWidget>(this, GameOverWidgetClass);

	if (GameOverWidget)
	{
		GameOverWidget->AddToViewport();
	}

	SetShowMouseCursor(true);

	FInputModeUIOnly InputMode;
	SetInputMode(InputMode);
}

void AOptimusPrimePlayerController::ClearAllWidgets()
{
	if (MainMenuWidgetInstance)
	{
		MainMenuWidgetInstance->RemoveFromParent();
		MainMenuWidgetInstance = nullptr;
	}

	if (HUDWidgetInstance)
	{
		HUDWidgetInstance->RemoveFromParent();
		HUDWidgetInstance = nullptr;
	}

	if (GameOverWidgetInstance)
	{
		GameOverWidgetInstance->RemoveFromParent();
		GameOverWidgetInstance = nullptr;
	}
}

void AOptimusPrimePlayerController::StartGame()
{
	UGameplayStatics::OpenLevel(
	    GetWorld(),
	    FName(TEXT("MainMap")));
}

void AOptimusPrimePlayerController::RetryGame()
{
	UGameplayStatics::OpenLevel(
	    GetWorld(),
	    FName(TEXT("MainMap")));
}

float AOptimusPrimePlayerController::GetAimRotationSpeed() const
{
	return AimRotationSpeed;
}
