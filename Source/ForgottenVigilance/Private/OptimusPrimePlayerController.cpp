#include "OptimusPrimePlayerController.h"
#include "OptimusPrimeCharacter.h"
#include "Blueprint/UserWidget.h"
#include "ForgottenMain.h"
#include "ForgottenHUDWidget.h"
#include "ForgottenGameOver.h"
#include "ForgottenMiniMap.h"
#include "ForgottenMiniMapCaptureActor.h"
#include "HealthComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"
#include "Kismet/GameplayStatics.h"
#include "ForgottenGameClear.h"

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
    , InteractAction(nullptr)
    , CrouchAction(nullptr)
    , DashAction(nullptr)
    , VisionStealAction(nullptr)
    , HUDWidgetClass(nullptr)
    , HUDWidgetInstance(nullptr)
    , MainMenuWidgetClass(nullptr)
    , MainMenuWidgetInstance(nullptr)
    , GameOverWidgetClass(nullptr)
    , GameOverWidgetInstance(nullptr)
    , InitialCameraPitch(DefaultInitialCameraPitch)
    , AimRotationSpeed(DefaultAimRotationSpeed)
    , GameClearWidgetClass(nullptr)
    , GameClearWidgetInstance(nullptr)
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
}

void AOptimusPrimePlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

void AOptimusPrimePlayerController::ShowMainMenu()
{
	if (!MainMenuWidgetClass)
	{
		return;
	}

	ClearAllWidgets();

	MainMenuWidgetInstance = CreateWidget<UForgottenMain>(this, MainMenuWidgetClass);

	if (MainMenuWidgetInstance)
	{
		MainMenuWidgetInstance->AddToViewport();
	}

	bShowMouseCursor = true;
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

	ClearAllWidgets();

	HUDWidgetInstance = CreateWidget<UForgottenHUDWidget>(this, HUDWidgetClass);

	if (HUDWidgetInstance)
	{
		HUDWidgetInstance->AddToViewport();
	}

	if (UForgottenMiniMap* MiniMapWidget = HUDWidgetInstance->GetMiniMapWidget())
	{
		AOptimusPrimeCharacter* MyCharacter = Cast<AOptimusPrimeCharacter>(GetPawn());

		AForgottenMiniMapCaptureActor* CaptureActor = Cast<AForgottenMiniMapCaptureActor>(
		    UGameplayStatics::GetActorOfClass(GetWorld(), AForgottenMiniMapCaptureActor::StaticClass()));

		if (MyCharacter && CaptureActor)
		{
			MiniMapWidget->InitMiniMap(CaptureActor, MyCharacter);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("ShowHUD: MiniMap init failed. Character=%s Capture=%s"),
			    *GetNameSafe(MyCharacter), *GetNameSafe(CaptureActor));
		}
	}

	bShowMouseCursor = false;
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

	ClearAllWidgets();

	UForgottenGameOver* GameOverWidget = CreateWidget<UForgottenGameOver>(this, GameOverWidgetClass);

	if (GameOverWidget)
	{
		GameOverWidget->AddToViewport();
	}

	bShowMouseCursor = true;
	SetShowMouseCursor(true);

	FInputModeUIOnly InputMode;
	SetInputMode(InputMode);
}

void AOptimusPrimePlayerController::ShowGameClear()
{
	if (!GameClearWidgetClass)
	{
		return;
	}

	ClearAllWidgets();

	GameClearWidgetInstance = CreateWidget<UForgottenGameClear>(this, GameClearWidgetClass);

	if (GameClearWidgetInstance)
	{
		GameClearWidgetInstance->AddToViewport();
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

	if (GameClearWidgetInstance)
	{
		GameClearWidgetInstance->RemoveFromParent();
		GameClearWidgetInstance = nullptr;
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
