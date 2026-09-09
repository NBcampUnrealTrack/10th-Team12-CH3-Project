<<<<<<< Updated upstream

#include "OptimusPrimeCharacter.h"
=======
﻿#include "OptimusPrimeCharacter.h"
>>>>>>> Stashed changes
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

	if (!FMath::IsNearlyZero(MoveInput.X))
	{
		AddMovementInput(GetActorForwardVector(), MoveInput.X);
	}
	if (!FMath::IsNearlyZero(MoveInput.Y))
	{
		AddMovementInput(GetActorRightVector(), MoveInput.Y);
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
	FVector2D LookInput = Value.Get<FVector2D>();

	AddControllerYawInput(LookInput.X);
	AddControllerPitchInput(LookInput.Y);
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
	}
}
