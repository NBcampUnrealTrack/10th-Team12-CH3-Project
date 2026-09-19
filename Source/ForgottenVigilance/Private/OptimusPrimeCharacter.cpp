#include "OptimusPrimeCharacter.h"
#include "Engine/LocalPlayer.h"
#include "SceneView.h"
#include "OptimusPrimePlayerController.h"
#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HealthComponent.h"
#include "VisionStealComponent.h"
#include "MainWeaponComponent.h"
#include "Animation/AnimMontage.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "TimerManager.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "ForgottenLever.h"

namespace
{
constexpr float DefaultCharacterTargetArmLength = 300.0f;
constexpr float DefaultNonForwardSpeedMultiplier = 0.5f;
constexpr float DefaultNormalSpeed = 600.0f;
constexpr float DefaultSprintMultiplier = 1.7f;
constexpr int32 RightHandMuzzleIndex = 0;
constexpr float DefaultHitStopDuration = 0.2f;
constexpr float DefaultHitStopTimeDilation = 0.05f;
constexpr float NormalTimeDilation = 1.0f;
constexpr float PlayerHealthZeroThreshold = 0.0f;

FVector CalculateDeadZonePivot(const FVector& PreviousPivot, const FVector& TargetLocation,
    const FVector& TargetMovement, const FVector& CameraRight, float HalfWidth)
{
	// 실제 이동 중 좌우 성분만 제외해 앞뒤 이동과 높이 변화는 즉시 추적.
	FVector Pivot = PreviousPivot + TargetMovement - CameraRight * FVector::DotProduct(TargetMovement, CameraRight);
	const float LateralDistance = FVector::DotProduct(TargetLocation - Pivot, CameraRight);
	const float ClampedDistance = FMath::Clamp(LateralDistance, -HalfWidth, HalfWidth);
	// 경계를 넘어간 거리만 보정하므로 정지 후 중앙으로 복귀하지 않음.
	return Pivot + CameraRight * (LateralDistance - ClampedDistance);
}
} // namespace

void AOptimusPrimeCharacter::EnableDeathRagdoll()
{
	// 살아 있거나 이미 래그돌 상태라면 처리 x
	if (!IsCharacterDead() || GetMesh()->IsSimulatingPhysics())
	{
		return;
	}

	// 이동용 캡슐 대신 메시의 물리 바디가 충돌을 담당
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));

	// 메시에 지정된 Physics Asset으로 물리 시뮬레이션을 시작
	GetMesh()->SetSimulatePhysics(true);
}

AOptimusPrimeCharacter::AOptimusPrimeCharacter()
    : NormalSpeed(DefaultNormalSpeed)
    , NonForwardSpeedMultiplier(DefaultNonForwardSpeedMultiplier)
    , SprintMultiplier(DefaultSprintMultiplier)
    , bIsMovingSidewaysOrBackward(false)
    , bIsSprinting(false)
    , CameraPivotWorldLocation(FVector::ZeroVector)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostPhysics;

	Tags.AddUnique(FName(TEXT("Player")));

	HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("Health"));
	MainWeaponComponent = CreateDefaultSubobject<UMainWeaponComponent>(TEXT("MainWeapon"));
	VisionStealComponent = CreateDefaultSubobject<UVisionStealComponent>(TEXT("VisionSteal"));

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComp->SetupAttachment(RootComponent);
	// C++ 기본값만 지정. 이후 BP의 스프링암 설정을 그대로 사용.
	SpringArmComp->TargetArmLength = DefaultCharacterTargetArmLength;
	SpringArmComp->bUsePawnControlRotation = true;
	SpringArmComp->SocketOffset.Z = 60.0f;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);
	CameraComp->bUsePawnControlRotation = false;

	// 몸은 화면 중앙의 조준 목표를 향해 별도로 회전.
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;

	GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;

	PawnSensingComp = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComp"));
	PawnSensingComp->SightRadius = 3000.0f;           
	PawnSensingComp->SetPeripheralVisionAngle(60.0f); 
	PawnSensingComp->HearingThreshold = 1200.0f;     
	PawnSensingComp->bOnlySensePlayers = false;      

	PawnSensingComp->OnSeePawn.AddDynamic(this, &AOptimusPrimeCharacter::OnPawnDetected);
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch= true;
	
	HitCameraShake = nullptr;
	HitStopDuration = DefaultHitStopDuration;
	HitStopTimeDilation = DefaultHitStopTimeDilation;
	PlayerPreviousHealth = PlayerHealthZeroThreshold;
}

void AOptimusPrimeCharacter::UpdateSpeed()
{
	if (bIsMovingSidewaysOrBackward)
	{
		GetCharacterMovement()->MaxWalkSpeed = NormalSpeed * NonForwardSpeedMultiplier;
	}
	else if (bIsSprinting)
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
	EndDash();
	GetCharacterMovement()->DisableMovement();

	if (MainWeaponComponent)
	{
		MainWeaponComponent->StopFire();
	}

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		DisableInput(PlayerController);
	}

	if (FMath::RandBool())
	{
		SelectedDeathSequence = ForwardDeathSequence;
	}
	else
	{
		SelectedDeathSequence = BackwardDeathSequence;
	}

	if (SelectedDeathSequence)
	{
		OnDeathAnimationReady.Broadcast();
	}
}

void AOptimusPrimeCharacter::HandleShotFired(int32 MuzzleIndex)
{
	UAnimMontage* SelectedMontage = MuzzleIndex == RightHandMuzzleIndex ? RightFireMontage : LeftFireMontage;

	if (!SelectedMontage)
	{
		return;
	}

	PlayAnimMontage(SelectedMontage, FireMontagePlayRate, NAME_None);
}

void AOptimusPrimeCharacter::UpdateDash(float DeltaTime)
{
	if (!bIsDashing)
	{
		return;
	}

	const float StepTime = FMath::Min(DeltaTime, DashDuration - DashElapsedTime);
	const float DashSpeed = DashDistance / DashDuration;
	const float StepDistance = DashSpeed * StepTime;

	FHitResult SweepHit;
	AddActorWorldOffset(DashDirection * StepDistance, true, &SweepHit);
	DashElapsedTime += StepTime;

	if (SweepHit.bBlockingHit || DashElapsedTime >= DashDuration)
	{
		EndDash();
	}
}

void AOptimusPrimeCharacter::EndDash()
{
	if (!bIsDashing)
	{
		return;
	}

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, SavedPawnCollisionResponse);
	bIsDashing = false;
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

	// 시작 기준점과 캐릭터 사이의 높이/위치 차이를 보존.
	CameraPivotWorldLocation = SpringArmComp->GetComponentLocation() + SpringArmComp->TargetOffset;

	InitialCameraPivotOffset = CameraPivotWorldLocation - GetActorLocation();
	PreviousCameraTargetLocation = GetActorLocation() + InitialCameraPivotOffset;
	// 이동 완료 -> 기준점 보정 -> 스프링암 갱신 순서로 한 프레임 지연을 피함.
	AddTickPrerequisiteComponent(GetCharacterMovement());
	SpringArmComp->AddTickPrerequisiteActor(this);
	SpringArmComp->bEnableCameraLag = false;
	SpringArmComp->bEnableCameraRotationLag = false;
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;

	UpdateSpeed();

	PlayerPreviousHealth = HealthComp->GetCurrentHealth();
	HealthComp->OnHealthChanged.AddDynamic(this, &AOptimusPrimeCharacter::HandleHealthChanged);
	MainWeaponComponent->OnShotFired.AddDynamic(this, &AOptimusPrimeCharacter::HandleShotFired);
}

void AOptimusPrimeCharacter::Move(const FInputActionValue& Value)
{
	if (!Controller)
	{
		return;
	}

	const FVector2D MoveInput = Value.Get<FVector2D>();
	CurrentMoveInput = MoveInput;
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
	if ((MoveInput.X < 0.0f && !FMath::IsNearlyZero(MoveInput.X)) || !FMath::IsNearlyZero(MoveInput.Y))
	{
		bIsMovingSidewaysOrBackward = true;
	}
	else
	{
		bIsMovingSidewaysOrBackward = false;
	}
	UpdateSpeed();
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

void AOptimusPrimeCharacter::UpdateCameraDeadZoneWidth(float DeltaTime)
{
	// 줌 중에도 현재 암 길이와 FOV로 폭을 갱신. 준비 전에는 일반 추적으로 대체.
	bDeadZoneWidthInitialized = false;

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

	// 현재 창 크기 대신 고정된 기준 화면을 사용해 뷰포트 크기 변경과 줌을 구분.
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
	if (HorizontalProjectionScale <= 0.0f)
	{
		return;
	}
	// 기준 화면 전체 폭에 대응하는 반폭을 저장. BP 비율은 아래에서 별도로 적용.
	// 벽 충돌로 줄어든 실제 거리가 아니라 줌이 설정한 암 길이를 사용.
	// 암 길이 0에서는 데드존도 0이 되어 캐릭터를 즉시 추적.
	DeadZoneReferenceHalfWidthWorld = FMath::Max(0.0f, SpringArm->TargetArmLength) / HorizontalProjectionScale;
	bDeadZoneWidthInitialized = true;
}

float AOptimusPrimeCharacter::GetCameraDeadZoneHalfWidth() const
{
	// 상한 없이 적용하므로 기준 화면보다 넓게 설정하거나 플레이 중 비율 변경 가능.
	return DeadZoneReferenceHalfWidthWorld * FMath::Max(0.0f, DeadZoneWidthFraction);
}

void AOptimusPrimeCharacter::UpdateCameraFollow()
{
	const FVector TargetLocation = GetActorLocation() + InitialCameraPivotOffset;
	const FVector TargetMovement = TargetLocation - PreviousCameraTargetLocation;
	const FRotator CameraYaw(0.0f, SpringArmComp->GetTargetRotation().Yaw, 0.0f);
	PreviousCameraTargetLocation = TargetLocation;
	if (!bDeadZoneWidthInitialized || TargetMovement.SizeSquared() > FMath::Square(CameraTeleportResetDistance))
	{
		// 초기화 전이나 순간이동 직후에는 이전 위치에 카메라를 남기지 않음.
		CameraPivotWorldLocation = TargetLocation;
	}
	else
	{
		const FVector RightDirection = FRotationMatrix(CameraYaw).GetUnitAxis(EAxis::Y);
		CameraPivotWorldLocation = CalculateDeadZonePivot(CameraPivotWorldLocation,
		    TargetLocation, TargetMovement, RightDirection, GetCameraDeadZoneHalfWidth());
	}
	// 부착된 스프링암의 이동을 보정해, 계산한 월드 기준점에서 카메라가 회전하도록 처리.
	SpringArmComp->TargetOffset = CameraPivotWorldLocation - SpringArmComp->GetComponentLocation();
}

void AOptimusPrimeCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 대시 이동 후 카메라가 변경된 위치를 따라가도록 먼저 갱신.
	UpdateDash(DeltaTime);

	UpdateCameraDeadZoneWidth(DeltaTime);

	UpdateCrosshairRotation(DeltaTime);
	UpdateCameraFollow();
}

void AOptimusPrimeCharacter::UpdateCrosshairRotation(float DeltaTime)
{
	const AOptimusPrimePlayerController* PlayerController = Cast<AOptimusPrimePlayerController>(GetController());
	if (!IsLocallyControlled() || !PlayerController || !MainWeaponComponent || IsCharacterDead())
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
		if (PlayerController->MoveAction)
		{
			EnhancedInput->BindAction(
			    PlayerController->MoveAction,
			    ETriggerEvent::Completed,
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
		if (PlayerController->DashAction)
		{
			EnhancedInput->BindAction(
			    PlayerController->DashAction,
			    ETriggerEvent::Started,
			    this,
			    &AOptimusPrimeCharacter::StartDash);
		}
		if (PlayerController->InteractAction)
		{
			EnhancedInput->BindAction(
			    PlayerController->InteractAction,
			    ETriggerEvent::Started,
			    this,
			    &AOptimusPrimeCharacter::Interact);
		}
		if (PlayerController->CrouchAction)
		{
			EnhancedInput->BindAction(
			    PlayerController->CrouchAction,
			    ETriggerEvent::Triggered,
			    this,
			    &AOptimusPrimeCharacter::StartCrouch);
		}
		if (PlayerController->CrouchAction)
		{
			EnhancedInput->BindAction(
			    PlayerController->CrouchAction,
			    ETriggerEvent::Completed,
			    this,
			    &AOptimusPrimeCharacter::StopCrouch);
		if (PlayerController->VisionStealAction)
		{
			EnhancedInput->BindAction(
			    PlayerController->VisionStealAction,
			    ETriggerEvent::Started,
			    this,
			    &AOptimusPrimeCharacter::UseVisionSteal);
		}
	}
}

void AOptimusPrimeCharacter::Interact(const FInputActionValue& Value)
{
	TArray<AActor*> OverlappingActors;
	GetOverlappingActors(OverlappingActors, AForgottenLever::StaticClass());

	for (AActor* OverlappingActor : OverlappingActors)
	{
		AForgottenLever* Lever = Cast<AForgottenLever>(OverlappingActor);

		if (!Lever || !Lever->CanInteract())
		{
			continue;
		}

		Lever->TryInteract();
		return;
	}
}

void AOptimusPrimeCharacter::StartCrouch(const FInputActionValue& Value)
{
	Crouch();
}

void AOptimusPrimeCharacter::StopCrouch(const FInputActionValue& Value)
{
	UnCrouch();
}

void AOptimusPrimeCharacter::StartDash(const FInputActionValue& Value)
{
	if (bIsDashing || bDashOnCooldown || IsCharacterDead() || !Controller)
	{
		return;
	}

	const FRotator DashRotation = Controller->GetControlRotation();
	const FRotator YawRotation = FRotator(0.0f, DashRotation.Yaw, 0.0f);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	if (CurrentMoveInput.IsNearlyZero())
	{
		DashDirection = ForwardDirection;
	}
	else
	{
		DashDirection = (ForwardDirection * CurrentMoveInput.X
		    + RightDirection * CurrentMoveInput.Y).GetSafeNormal();
	}

	SavedPawnCollisionResponse = GetCapsuleComponent()->GetCollisionResponseToChannel(ECC_Pawn);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	DashElapsedTime = 0.0f;
	bIsDashing = true;

	if (DashCooldown > 0.0f)
	{
		bDashOnCooldown = true;
		GetWorldTimerManager().SetTimer(
		    DashCooldownTimer,
		    FTimerDelegate::CreateWeakLambda(this, [this]() {
			    bDashOnCooldown = false;

			    if (GEngine && !IsCharacterDead())
			    {
				    GEngine->AddOnScreenDebugMessage(
				        -1, 1.5f, FColor::Green, TEXT("대시 사용 가능"));
			    }
		    }),
		    DashCooldown,
		    false);
	}
void AOptimusPrimeCharacter::UseVisionSteal(const FInputActionValue& Value)
{
	VisionStealComponent->ToggleVisionSteal();
}

void AOptimusPrimeCharacter::OnPawnDetected(APawn* DetectedPawn)
{
	if (!DetectedPawn || DetectedPawn == this)
	{
		return;
	}

	const bool bIsNew = !DetectedEnemies.Contains(DetectedPawn);

	DetectedEnemies.Add(DetectedPawn, GetWorld()->GetTimeSeconds());

	if (bIsNew)
	{
		OnTargetDetected.Broadcast(DetectedPawn);
	}
}

AActor* AOptimusPrimeCharacter::GetCurrentTarget() const
{
	return CurrentTarget;
}

void AOptimusPrimeCharacter::ClearCurrentTarget()
{
	CurrentTarget = nullptr;
}

void AOptimusPrimeCharacter::GetDetectedTargets(TArray<AActor*>& OutTargets) const
{
	for (const TPair<TWeakObjectPtr<APawn>, float>& Pair : DetectedEnemies)
	{
		if (APawn* Pawn = Pair.Key.Get())
		{
			OutTargets.Add(Pawn);
		}
	}
}

void AOptimusPrimeCharacter::CheckStaleTargets()
{
	const float CurrentTime = GetWorld()->GetTimeSeconds();

	TArray<TWeakObjectPtr<APawn>> StaleTargets;

	for (const TPair<TWeakObjectPtr<APawn>, float>& Pair : DetectedEnemies)
	{
		if (!Pair.Key.IsValid() || CurrentTime - Pair.Value > 5.0f)
		{
			StaleTargets.Add(Pair.Key);
		}
	}


	for (const TWeakObjectPtr<APawn>& StaleTarget : StaleTargets)
	{
		if (APawn* Pawn = StaleTarget.Get())
		{
			OnTargetLost.Broadcast(Pawn);
		}
		DetectedEnemies.Remove(StaleTarget);
	}
}

void AOptimusPrimeCharacter::HandleHealthChanged(float CurrentHealth, float MaxHealth)
{
	const bool bDamaged = CurrentHealth < PlayerPreviousHealth;
	PlayerPreviousHealth = CurrentHealth;

	if (!bDamaged || CurrentHealth <= PlayerHealthZeroThreshold)
	{
		return;
	}

	OnPlayerDamaged.Broadcast();

	APlayerController* OwningController = Cast<APlayerController>(GetController());

	if (OwningController && HitCameraShake)
	{
		OwningController->ClientStartCameraShake(HitCameraShake);
	}

	if (HitStopDuration <= PlayerHealthZeroThreshold)
	{
		return;
	}

	UGameplayStatics::SetGlobalTimeDilation(this, HitStopTimeDilation);

	GetWorldTimerManager().SetTimer(
		HitStopTimerHandle,
		this,
		&AOptimusPrimeCharacter::EndHitStop,
		HitStopDuration * HitStopTimeDilation,
		false);
}

void AOptimusPrimeCharacter::EndHitStop()
{
	UGameplayStatics::SetGlobalTimeDilation(this, NormalTimeDilation);
}
