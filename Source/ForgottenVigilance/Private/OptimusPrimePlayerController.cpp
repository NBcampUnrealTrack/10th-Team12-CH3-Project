#include "OptimusPrimePlayerController.h"
#include "Blueprint/UserWidget.h"
#include "DrawDebugHelpers.h"
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
    , InitialCameraPitch(DefaultInitialCameraPitch)
    , AimRotationSpeed(DefaultAimRotationSpeed)
{
}

void AOptimusPrimePlayerController::BeginPlay()
{
	Super::BeginPlay();

	SetControlRotation(FRotator(InitialCameraPitch, GetControlRotation().Yaw, 0.0f));

	SetShowMouseCursor(true);
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);

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
		UUserWidget* HUDWidget = CreateWidget<UUserWidget>(this, HUDWidgetClass);
		if (HUDWidget)
		{
			HUDWidget->AddToViewport();
		}
	}
}

void AOptimusPrimePlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FHitResult CursorHit;

	const bool bHit = GetHitResultUnderCursorByChannel(
	    UEngineTypes::ConvertToTraceType(ECC_Visibility),
	    false,
	    CursorHit);

	if (bHit)
	{
		DrawDebugSphere(
		    GetWorld(),
		    CursorHit.ImpactPoint,
		    1.0f,
		    12,
		    FColor::Red,
		    false,
		    0.1f);

		APawn* ControlledPawn = GetPawn();
		if (!ControlledPawn)
		{
			return;
		}
		const FVector AimDirection = CursorHit.ImpactPoint - ControlledPawn->GetActorLocation();
		const FRotator AimRotation = AimDirection.Rotation();

		const FRotator NextRotation = FMath::RInterpConstantTo(
		    ControlledPawn->GetActorRotation(),
		    FRotator(0.0f, AimRotation.Yaw, 0.0f),
		    DeltaTime,
		    AimRotationSpeed);

		ControlledPawn->SetActorRotation(NextRotation);
	}
}
