#include "OptimusPrimeCharacter.h"
#include "OptimusPrimePlayerController.h"
#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HealthComponent.h"
#include "MainWeaponComponent.h"

AOptimusPrimeCharacter::AOptimusPrimeCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	Tags.AddUnique(FName(TEXT("Player")));

	HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("Health"));
	MainWeaponComponent = CreateDefaultSubobject<UMainWeaponComponent>(TEXT("MainWeapon"));

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->TargetArmLength = 1200.0f;
	SpringArmComp->bUsePawnControlRotation = true;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);
	CameraComp->bUsePawnControlRotation = false;

	// 몸은 컨트롤러의 시선 대신 이동 방향을 따라 회전
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	bIsCameraRotating = false;

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

void AOptimusPrimeCharacter::HandleDeath(AActor* DeadOwner)
{
	GetCharacterMovement()->DisableMovement();
}

bool AOptimusPrimeCharacter::IsCharacterDead() const
{
	if (!HealthComp)
	{
		return false;
	}

	return !(HealthComp->IsAlive());
}

void AOptimusPrimeCharacter::BeginPlay()
{
	Super::BeginPlay();

	UpdateSpeed();

	HealthComp->OnDeath.AddDynamic(this, &AOptimusPrimeCharacter::HandleDeath);
}

void AOptimusPrimeCharacter::Move(const FInputActionValue& Value)
{
	if (!Controller)
	{
		return;
	}

	const FVector2D MoveInput = Value.Get<FVector2D>();
	const FRotator ControlRotation = Controller->GetControlRotation();
	const FRotator YawRotation = FRotator(0.0f, ControlRotation.Yaw, 0.0f);

	if (!FMath::IsNearlyZero(MoveInput.X))
	{
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(ForwardDirection, MoveInput.X);
	}
	if (!FMath::IsNearlyZero(MoveInput.Y))
	{
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(RightDirection, MoveInput.Y);
	}
}

void AOptimusPrimeCharacter::StartJump(const FInputActionValue& Value)
{
	if (Value.Get<bool>())
	{
		Jump();
	}
}

void AOptimusPrimeCharacter::StopJump(const FInputActionValue& Value)
{
	if (!Value.Get<bool>())
	{
		StopJumping();
	}
}

void AOptimusPrimeCharacter::Look(const FInputActionValue& Value)
{
	if (bIsCameraRotating)
	{
		const FVector2D LookInput = Value.Get<FVector2D>();

		AddControllerYawInput(LookInput.X);
		AddControllerPitchInput(LookInput.Y);
	}
}

void AOptimusPrimeCharacter::StartSprint(const FInputActionValue& Value)
{
	bIsSprinting = true;
	UpdateSpeed();
}

void AOptimusPrimeCharacter::StopSprint(const FInputActionValue& Value)
{
	bIsSprinting = false;
	UpdateSpeed();
}

void AOptimusPrimeCharacter::FireWeapon(const FInputActionValue& Value)
{
	MainWeaponComponent->StartFire();
}

void AOptimusPrimeCharacter::StopFireWeapon(const FInputActionValue& Value)
{
	MainWeaponComponent->StopFire();
}

void AOptimusPrimeCharacter::StartCameraRotate(const FInputActionValue& Value)
{
	bIsCameraRotating = true;
}

void AOptimusPrimeCharacter::StopCameraRotate(const FInputActionValue& Value)
{
	bIsCameraRotating = false;
}

void AOptimusPrimeCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AOptimusPrimeCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInput)
	{
		return;
	}

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
		if (PlayerController->ShootAction)
		{
			EnhancedInput->BindAction(
			    PlayerController->ShootAction,
			    ETriggerEvent::Triggered,
			    this,
			    &AOptimusPrimeCharacter::FireWeapon);
		}
		if (PlayerController->ShootAction)
		{
			EnhancedInput->BindAction(
			    PlayerController->ShootAction,
			    ETriggerEvent::Completed,
			    this,
			    &AOptimusPrimeCharacter::StopFireWeapon);
		}
		if (PlayerController->CameraRotateAction)
		{
			EnhancedInput->BindAction(
			    PlayerController->CameraRotateAction,
			    ETriggerEvent::Started,
			    this,
			    &AOptimusPrimeCharacter::StartCameraRotate);
		}
		if (PlayerController->CameraRotateAction)
		{
			EnhancedInput->BindAction(
			    PlayerController->CameraRotateAction,
			    ETriggerEvent::Completed,
			    this,
			    &AOptimusPrimeCharacter::StopCameraRotate);
		}
	}
}
