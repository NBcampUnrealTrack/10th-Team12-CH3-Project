#include "OptimusPrimeCharacter.h"
#include "Engine/LocalPlayer.h"
#include "SceneView.h"
#include "OptimusPrimePlayerController.h"
#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HealthComponent.h"
#include "MainWeaponComponent.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#endif

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCameraDeadZoneMovementTest,
	"ForgottenVigilance.Camera.DeadZoneMovement",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCameraScreenPositionTest,
	"ForgottenVigilance.Camera.DeadZoneScreenPosition",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
#endif

namespace
{
constexpr float DefaultCharacterTargetArmLength = 500.0f;

FVector RotateCameraPivotWithYaw(const FVector& PreviousPivot, const FVector& CharacterLocation, float DeltaYaw)
{
	// 캐릭터는 이동시키지 않고 기준점의 상대 위치를 카메라 Yaw 변화만큼 회전합니다.
	return CharacterLocation + (PreviousPivot - CharacterLocation).RotateAngleAxis(DeltaYaw, FVector::UpVector);
}

FVector CalculateDeadZonePivot(const FVector& PreviousPivot, const FVector& TargetLocation,
	const FVector& TargetMovement, const FVector& CameraRight, float HalfWidth)
{
	// 실제 이동 중 좌우 성분만 제외해 앞뒤 이동과 높이 변화는 즉시 따라갑니다.
	FVector Pivot = PreviousPivot + TargetMovement
		- CameraRight * FVector::DotProduct(TargetMovement, CameraRight);
	const float LateralDistance = FVector::DotProduct(TargetLocation - Pivot, CameraRight);
	const float ClampedDistance = FMath::Clamp(LateralDistance, -HalfWidth, HalfWidth);
	// 경계를 넘어간 거리만 보정하므로 정지 후 중앙으로 돌아가지 않습니다.
	return Pivot + CameraRight * (LateralDistance - ClampedDistance);
}
} // namespace

#if WITH_DEV_AUTOMATION_TESTS
bool FCameraDeadZoneMovementTest::RunTest(const FString& Parameters)
{
	const FVector Right(0.0, 1.0, 0.0);
	const float HalfWidth = 250.0f;
	FVector Pivot = CalculateDeadZonePivot(FVector::ZeroVector, FVector(0, 100, 0),
		FVector(0, 100, 0), Right, HalfWidth);
	TestTrue(TEXT("Inside: lateral movement leaves pivot fixed"), Pivot.IsNearlyZero());
	Pivot = CalculateDeadZonePivot(Pivot, FVector(0, 300, 0), FVector(0, 200, 0), Right, HalfWidth);
	TestTrue(TEXT("Right boundary: follow only the 50 cm excess"), Pivot.Equals(FVector(0, 50, 0)));
	Pivot = CalculateDeadZonePivot(Pivot, FVector(0, 300, 0), FVector::ZeroVector, Right, HalfWidth);
	TestTrue(TEXT("Stop: retain boundary position"), Pivot.Equals(FVector(0, 50, 0)));
	Pivot = CalculateDeadZonePivot(Pivot, FVector(0, 200, 0), FVector(0, -100, 0), Right, HalfWidth);
	TestTrue(TEXT("Reverse: travel inside without recentering"), Pivot.Equals(FVector(0, 50, 0)));
	Pivot = CalculateDeadZonePivot(Pivot, FVector(100, 200, 80), FVector(100, 0, 80), Right, HalfWidth);
	TestTrue(TEXT("Forward and jump: follow both immediately"), Pivot.Equals(FVector(100, 50, 80)));
	Pivot = CalculateDeadZonePivot(Pivot, FVector(0, 200, 0), FVector(-100, 0, -80), Right, HalfWidth);
	TestTrue(TEXT("Backward and falling: follow both immediately"), Pivot.Equals(FVector(0, 50, 0)));
	Pivot = CalculateDeadZonePivot(FVector::ZeroVector, FVector(0, -300, 0),
		FVector(0, -300, 0), Right, HalfWidth);
	TestTrue(TEXT("Left boundary is symmetric"), Pivot.Equals(FVector(0, -50, 0)));
	const FVector RotatedRight(-1, 0, 0);
	Pivot = CalculateDeadZonePivot(FVector::ZeroVector, FVector(-100, 0, 0),
		FVector(-100, 0, 0), RotatedRight, HalfWidth);
	TestTrue(TEXT("Yaw 90: world X lateral motion stays inside"), Pivot.IsNearlyZero());
	Pivot = CalculateDeadZonePivot(Pivot, FVector(-100, 120, 0), FVector(0, 120, 0), RotatedRight, HalfWidth);
	TestTrue(TEXT("Yaw 90: world Y forward motion follows"), Pivot.Equals(FVector(0, 120, 0)));
	Pivot = CalculateDeadZonePivot(FVector::ZeroVector, FVector(0, 200, 0),
		FVector::ZeroVector, RotatedRight, HalfWidth);
	TestTrue(TEXT("Mouse-only orbit inside bounds keeps pivot fixed"), Pivot.IsNearlyZero());
	for (const int32 FrameCount : {30, 60, 144})
	{
		Pivot = FVector::ZeroVector;
		FVector PreviousTarget = FVector::ZeroVector;
		for (int32 Frame = 1; Frame <= FrameCount; ++Frame)
		{
			const FVector Target(400.0 * Frame / FrameCount, 1000.0 * Frame / FrameCount, 0);
			Pivot = CalculateDeadZonePivot(Pivot, Target, Target - PreviousTarget, Right, HalfWidth);
			PreviousTarget = Target;
		}
		TestTrue(FString::Printf(TEXT("Diagonal travel at %d FPS has same final pivot"), FrameCount),
			Pivot.Equals(FVector(400, 750, 0), 0.01));
	}
	return true;
}

bool FCameraScreenPositionTest::RunTest(const FString& Parameters)
{
	const float HalfWidth = 250.0f;
	const float ArmLength = 500.0f;
	const FVector CharacterLocation(0, HalfWidth, 0);
	const FVector InitialPivot = FVector::ZeroVector;
	const float InitialPitch = -30.0f;
	const auto ScreenX = [ArmLength](const FVector& Target, const FVector& Pivot, const FRotator& Rotation)
	{
		const FVector CameraLocation = Pivot - Rotation.Vector() * ArmLength;
		const FVector Relative = Target - CameraLocation;
		// 수평 FOV 90도인 화면에서의 정규화된 X 좌표입니다.
		return 0.5 + 0.5 * FVector::DotProduct(Relative, FRotationMatrix(Rotation).GetUnitAxis(EAxis::Y))
			/ FVector::DotProduct(Relative, Rotation.Vector());
	};
	const FRotator InitialRotation(InitialPitch, 0, 0);
	const double InitialScreenX = ScreenX(CharacterLocation, InitialPivot, InitialRotation);
	const double InitialDistance = FVector::Distance(CharacterLocation, InitialPivot - InitialRotation.Vector() * ArmLength);
	TestTrue(TEXT("Start at screen right quarter boundary"), FMath::IsNearlyEqual(InitialScreenX, 0.75));
	for (const float Yaw : {-90.0f, 90.0f, 180.0f, 270.0f})
	{
		const FRotator Rotation(InitialPitch, Yaw, 0);
		const FVector Right = FRotationMatrix(Rotation).GetUnitAxis(EAxis::Y);
		const FVector RotatedPivot = RotateCameraPivotWithYaw(InitialPivot, CharacterLocation, Yaw);
		const FVector Pivot = CalculateDeadZonePivot(RotatedPivot, CharacterLocation, FVector::ZeroVector, Right, HalfWidth);
		TestTrue(FString::Printf(TEXT("Yaw %.0f keeps screen position"), Yaw),
			FMath::IsNearlyEqual(ScreenX(CharacterLocation, Pivot, Rotation), InitialScreenX, 0.0001));
		TestTrue(FString::Printf(TEXT("Yaw %.0f keeps camera distance"), Yaw),
			FMath::IsNearlyEqual(FVector::Distance(CharacterLocation, Pivot - Rotation.Vector() * ArmLength), InitialDistance, 0.001));
	}
	const FRotator LeftRotation(InitialPitch, -90.0f, 0);
	TestTrue(TEXT("Old fixed pivot moves character to screen center in comparison diagram"),
		FMath::IsNearlyEqual(ScreenX(CharacterLocation, InitialPivot, LeftRotation), 0.5, 0.0001));
	TestTrue(TEXT("Yaw wrap is a 2 degree turn"),
		FMath::IsNearlyEqual(FMath::FindDeltaAngleDegrees(179.0f, -179.0f), 2.0f));
	for (const int32 FrameCount : {30, 60, 144})
	{
		FVector Target = FVector::ZeroVector;
		FVector Pivot = FVector::ZeroVector;
		float PreviousYaw = 0.0f;
		for (int32 Frame = 1; Frame <= FrameCount; ++Frame)
		{
			const float Yaw = -90.0f * Frame / FrameCount;
			const FRotator Rotation(0, Yaw, 0);
			const FVector Right = FRotationMatrix(Rotation).GetUnitAxis(EAxis::Y);
			const FVector Movement = (Right * 600.0 + Rotation.Vector() * 200.0 + FVector(0, 0, 50)) / FrameCount;
			Pivot = RotateCameraPivotWithYaw(Pivot, Target, FMath::FindDeltaAngleDegrees(PreviousYaw, Yaw));
			Target += Movement;
			Pivot = CalculateDeadZonePivot(Pivot, Target, Movement, Right, HalfWidth);
			PreviousYaw = Yaw;
		}
		TestTrue(FString::Printf(TEXT("Simultaneous move, jump and yaw at %d FPS keeps final boundary"), FrameCount),
			FMath::IsNearlyEqual(ScreenX(Target, Pivot, LeftRotation), 0.75, 0.0001));
	}
	return true;
}
#endif

AOptimusPrimeCharacter::AOptimusPrimeCharacter()
    : CharacterTargetArmLength(DefaultCharacterTargetArmLength)
    , CameraPivotWorldLocation(0.0f)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostPhysics;

	Tags.AddUnique(FName(TEXT("Player")));

	HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("Health"));
	MainWeaponComponent = CreateDefaultSubobject<UMainWeaponComponent>(TEXT("MainWeapon"));

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->TargetArmLength = CharacterTargetArmLength;
	SpringArmComp->bUsePawnControlRotation = true;
	
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);
	CameraComp->bUsePawnControlRotation = false;
	
	// 몸은 화면 중앙의 조준 목표를 향해 별도로 회전합니다.
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;

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
	
	CameraPivotWorldLocation = SpringArmComp->GetComponentLocation() + SpringArmComp->TargetOffset;

	// BP에서 변경한 값 반영
	SpringArmComp->TargetArmLength = CharacterTargetArmLength;

	InitialCameraPivotOffset = CameraPivotWorldLocation - GetActorLocation();
	PreviousCameraTargetLocation = GetActorLocation() + InitialCameraPivotOffset;
	PreviousCameraYaw = SpringArmComp->GetTargetRotation().Yaw;
	// 이동 완료 -> 기준점 보정 -> 스프링암 갱신 순서로 한 프레임 지연을 피합니다.
	AddTickPrerequisiteComponent(GetCharacterMovement());
	SpringArmComp->AddTickPrerequisiteActor(this);
	SpringArmComp->bEnableCameraLag = false;
	SpringArmComp->bEnableCameraRotationLag = false;
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;

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
	const FVector2D LookInput = Value.Get<FVector2D>();

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

void AOptimusPrimeCharacter::InitializeCameraDeadZone(float DeltaTime)
{
	if (bDeadZoneWidthInitialized)
	{
		return;
	}

	const APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController || !PlayerController->GetLocalPlayer())
	{
		return;
	}
	UCameraComponent* Camera = CameraComp.Get();
	const USpringArmComponent* SpringArm = SpringArmComp.Get();
	if (!Camera || !SpringArm || DeadZoneReferenceResolution.X <= 0 || DeadZoneReferenceResolution.Y <= 0)
	{
		return;
	}

	// 현재 창 크기 대신 기준 화면의 투영으로 폭을 한 번 계산합니다.
	FMinimalViewInfo ReferenceView;
	Camera->GetCameraView(DeltaTime, ReferenceView);
	if (ReferenceView.ProjectionMode != ECameraProjectionMode::Perspective)
	{
		return;
	}
	FSceneViewProjectionData ReferenceProjection;
	const FIntRect ReferenceRect(0, 0, DeadZoneReferenceResolution.X, DeadZoneReferenceResolution.Y);
	ReferenceProjection.SetViewRectangle(ReferenceRect);
	FMinimalViewInfo::CalculateProjectionMatrixGivenViewRectangle(ReferenceView,
		PlayerController->GetLocalPlayer()->AspectRatioAxisConstraint, ReferenceRect, ReferenceProjection);
	const float HorizontalProjectionScale = ReferenceProjection.ProjectionMatrix.M[0][0];
	if (HorizontalProjectionScale <= 0.0f || SpringArm->TargetArmLength <= 0.0f)
	{
		return;
	}
	// 충돌로 일시적으로 짧아진 카메라 거리가 아닌 기본 암 길이를 기준으로 고정합니다.
	DeadZoneHalfWidthWorld = SpringArm->TargetArmLength
		* FMath::Clamp(ReferenceDeadZoneScreenFraction, 0.0f, 1.0f) / HorizontalProjectionScale;
	bDeadZoneWidthInitialized = true;
}

float AOptimusPrimeCharacter::GetCameraDeadZoneHalfWidth() const
{
	return DeadZoneHalfWidthWorld * FMath::Max(0.0f, DeadZoneWidthScale);
}

bool AOptimusPrimeCharacter::GetCameraDeadZoneBounds(FVector& LeftBoundary, FVector& RightBoundary) const
{
	if (CameraMode == EPlayerCameraMode::CharacterOrbit || !bDeadZoneWidthInitialized || !SpringArmComp)
	{
		return false;
	}
	const FRotator CameraYaw(0.0f, SpringArmComp->GetTargetRotation().Yaw, 0.0f);
	const FVector ForwardDirection = FRotationMatrix(CameraYaw).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(CameraYaw).GetUnitAxis(EAxis::Y);
	const FVector TargetLocation = GetActorLocation() + InitialCameraPivotOffset;
	// 캐릭터가 있는 깊이에서 경계를 표시해 공전 중에도 판정 위치와 표시가 맞도록 합니다.
	FVector BoundaryCenter = CameraPivotWorldLocation + ForwardDirection
		* FVector::DotProduct(TargetLocation - CameraPivotWorldLocation, ForwardDirection);
	BoundaryCenter.Z = TargetLocation.Z;
	const float HalfWidth = GetCameraDeadZoneHalfWidth();
	LeftBoundary = BoundaryCenter - RightDirection * HalfWidth;
	RightBoundary = BoundaryCenter + RightDirection * HalfWidth;
	return true;
}

void AOptimusPrimeCharacter::UpdateCameraFollow()
{
	const FVector TargetLocation = GetActorLocation() + InitialCameraPivotOffset;
	const FVector TargetMovement = TargetLocation - PreviousCameraTargetLocation;
	const FVector PreviousCharacterLocation = PreviousCameraTargetLocation - InitialCameraPivotOffset;
	const FRotator CameraYaw(0.0f, SpringArmComp->GetTargetRotation().Yaw, 0.0f);
	const float DeltaYaw = FMath::FindDeltaAngleDegrees(PreviousCameraYaw, CameraYaw.Yaw);
	PreviousCameraTargetLocation = TargetLocation;
	PreviousCameraYaw = CameraYaw.Yaw;
	if (CameraMode == EPlayerCameraMode::CharacterOrbit)
	{
		// 캐릭터의 수평 중심을 공전 기준으로 사용하고 기존 카메라 높이는 유지합니다.
		CameraPivotWorldLocation = GetActorLocation() + FVector(0.0f, 0.0f, InitialCameraPivotOffset.Z);
	}
	else if (!bDeadZoneWidthInitialized
		|| TargetMovement.SizeSquared() > FMath::Square(CameraTeleportResetDistance))
	{
		CameraPivotWorldLocation = TargetLocation;
	}
	else
	{
		if (CameraMode == EPlayerCameraMode::ScreenPositionDeadZone)
		{
			CameraPivotWorldLocation = RotateCameraPivotWithYaw(CameraPivotWorldLocation,
				PreviousCharacterLocation, DeltaYaw);
		}
		const FVector RightDirection = FRotationMatrix(CameraYaw).GetUnitAxis(EAxis::Y);
		CameraPivotWorldLocation = CalculateDeadZonePivot(CameraPivotWorldLocation,
			TargetLocation, TargetMovement, RightDirection, GetCameraDeadZoneHalfWidth());
	}
	SpringArmComp->TargetOffset = CameraPivotWorldLocation - SpringArmComp->GetComponentLocation();
}

void AOptimusPrimeCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	InitializeCameraDeadZone(DeltaTime);

	UpdateCrosshairRotation(DeltaTime);
	UpdateCameraFollow();
}

void AOptimusPrimeCharacter::UpdateCrosshairRotation(float DeltaTime)
{
	const AOptimusPrimePlayerController* PlayerController = Cast<AOptimusPrimePlayerController>(GetController());
	if (!bRotateTowardCrosshair || !IsLocallyControlled() || !PlayerController
		|| !MainWeaponComponent || IsCharacterDead())
	{
		return;
	}
	FVector AimTarget;
	if (!MainWeaponComponent->GetAimTarget(AimTarget))
	{
		return;
	}
	FVector AimDirection = AimTarget - GetActorLocation();
	AimDirection.Z = 0.0f;
	if (AimDirection.IsNearlyZero())
	{
		return;
	}
	const FRotator TargetRotation(0.0f, AimDirection.Rotation().Yaw, 0.0f);
	const FRotator NextRotation = FMath::RInterpConstantTo(GetActorRotation(), TargetRotation,
		DeltaTime, FMath::Max(0.0f, PlayerController->GetAimRotationSpeed()));
	SetActorRotation(NextRotation);
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
