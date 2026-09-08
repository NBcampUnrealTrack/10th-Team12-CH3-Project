
#include "OptimusPrimeCharacter.h"
#include "OptimusPrimePlayerController.h"
#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

AOptimusPrimeCharacter::AOptimusPrimeCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->TargetArmLength = 300.0f;
	SpringArmComp->bUsePawnControlRotation = true;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);
	CameraComp->bUsePawnControlRotation = false;

	bIsSprinting = false;
	NormalSpeed = 600.0f;
	SprintMultiplier = 1.7f;

	GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;
}

void AOptimusPrimeCharacter::UpdateSpeed()
{
	if (bIsSprinting)
	{
		GetCharacterMovement()->MaxWalkSpeed = NormalSpeed * SprintMultiplier;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;
	}
}

void AOptimusPrimeCharacter::BeginPlay()
{
	Super::BeginPlay();

	UpdateSpeed();
}

void AOptimusPrimeCharacter::Move(const FInputActionValue& value)
{
	if (!Controller)
		return;

	const FVector2D Moveinput = value.Get<FVector2D>();

	if (!FMath::IsNearlyZero(Moveinput.X))
	{
		AddMovementInput(GetActorForwardVector(), Moveinput.X);
	}
	if (!FMath::IsNearlyZero(Moveinput.Y))
	{
		AddMovementInput(GetActorRightVector(), Moveinput.Y);
	}
}

void AOptimusPrimeCharacter::StartJump(const FInputActionValue& value)
{
	if (value.Get<bool>())
	{
		Jump();
	}
}

void AOptimusPrimeCharacter::StopJump(const FInputActionValue& value)
{
	if (!value.Get<bool>())
	{
		StopJumping();
	}
}

void AOptimusPrimeCharacter::Look(const FInputActionValue& value)
{
	FVector2D LookInput = value.Get<FVector2D>();

	AddControllerYawInput(LookInput.X);
	AddControllerPitchInput(LookInput.Y);
}

void AOptimusPrimeCharacter::StartSprint(const FInputActionValue& value)
{
	bIsSprinting = true;
	UpdateSpeed();
}

void AOptimusPrimeCharacter::StopSprint(const FInputActionValue& value)
{
	bIsSprinting = false;
	UpdateSpeed();
}

void AOptimusPrimeCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AOptimusPrimeCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (AOptimusPrimePlayerController* PlayerController = Cast<AOptimusPrimePlayerController>(GetController()))
		{
			if (PlayerController->MoveAction)
			{
				EnhancedInput->BindAction(
				    PlayerController->MoveAction,
				    ETriggerEvent::Triggered,
				    this,
				    &AOptimusPrimeCharacter::Move);
			}
			if (PlayerController->LookAction)
			{
				EnhancedInput->BindAction(
				    PlayerController->LookAction,
				    ETriggerEvent::Triggered,
				    this,
				    &AOptimusPrimeCharacter::Look);
			}
			if (PlayerController->JumpAction)
			{
				EnhancedInput->BindAction(
				    PlayerController->JumpAction,
				    ETriggerEvent::Triggered,
				    this,
				    &AOptimusPrimeCharacter::StartJump);
			}
			if (PlayerController->JumpAction)
			{
				EnhancedInput->BindAction(
				    PlayerController->JumpAction,
				    ETriggerEvent::Completed,
				    this,
				    &AOptimusPrimeCharacter::StopJump);
			}
			if (PlayerController->SprintAction)
			{
				EnhancedInput->BindAction(
				    PlayerController->SprintAction,
				    ETriggerEvent::Triggered,
				    this,
				    &AOptimusPrimeCharacter::StartSprint);
			}
			if (PlayerController->SprintAction)
			{
				EnhancedInput->BindAction(
				    PlayerController->SprintAction,
				    ETriggerEvent::Completed,
				    this,
				    &AOptimusPrimeCharacter::StopSprint);
			}
		}
	}
}

