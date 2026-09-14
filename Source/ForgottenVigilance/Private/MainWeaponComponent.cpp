#include "MainWeaponComponent.h"

#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"

namespace
{
constexpr float DefaultDamage = 50.0f;
constexpr float DefaultFireInterval = 0.1f;
constexpr float DefaultTraceDistance = 10000.0f;
constexpr float DefaultMaxHeat = 100.0f;
constexpr float DefaultHeatPerShot = 1.0f;
constexpr float DefaultCoolingRate = 25.0f;
constexpr float ZeroThreshold = 0.0f;
const FName DefaultAttachSocketName(TEXT("weapon"));
const FName DefaultMuzzleSocketName(TEXT("Muzzle"));

bool TraceMuzzlePath(UWorld* World, const FVector& GuardStart, const FVector& MuzzleStart,
	const FVector& TraceEnd, const FCollisionQueryParams& QueryParams, FHitResult& OutHit, bool& bMuzzleBlocked)
{
	bMuzzleBlocked = World->LineTraceSingleByChannel(OutHit, GuardStart, MuzzleStart,
		ECC_Visibility, QueryParams);
	return bMuzzleBlocked || World->LineTraceSingleByChannel(OutHit, MuzzleStart,
		TraceEnd, ECC_Visibility, QueryParams);
}
}

UMainWeaponComponent::UMainWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	WeaponMesh = nullptr;
	WeaponMeshComponent = nullptr;
	AttachSocketName = DefaultAttachSocketName;
	MuzzleSocketName = DefaultMuzzleSocketName;

	Damage = DefaultDamage;
	FireInterval = DefaultFireInterval;
	TraceDistance = DefaultTraceDistance;
	MaxHeat = DefaultMaxHeat;
	HeatPerShot = DefaultHeatPerShot;
	CoolingRate = DefaultCoolingRate;
	CurrentHeat = ZeroThreshold;
	bIsOverheated = false;
}

void UMainWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentHeat = ZeroThreshold;
	bIsOverheated = false;
	OnHeatChanged.Broadcast(CurrentHeat, MaxHeat);

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter == nullptr)
	{
		return;
	}

	AttachToCharacterMesh(OwnerCharacter->GetMesh());
	if (!WeaponMeshComponent || !WeaponMeshComponent->DoesSocketExist(MuzzleSocketName))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: muzzle socket '%s' is unavailable; firing from pawn view location."),
			*GetNameSafe(GetOwner()), *MuzzleSocketName.ToString());
	}
}

void UMainWeaponComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateCooling(DeltaTime);
}

void UMainWeaponComponent::StartFire()
{
	if (bIsOverheated)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	if (World->GetTimerManager().IsTimerActive(FireTimerHandle))
	{
		return;
	}

	Fire();
	World->GetTimerManager().SetTimer(FireTimerHandle, this, &UMainWeaponComponent::Fire, FireInterval, true);
}

void UMainWeaponComponent::StopFire()
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	World->GetTimerManager().ClearTimer(FireTimerHandle);
}

bool UMainWeaponComponent::IsOverheated() const
{
	return bIsOverheated;
}

float UMainWeaponComponent::GetHeatRatio() const
{
	if (MaxHeat <= ZeroThreshold)
	{
		return ZeroThreshold;
	}

	return CurrentHeat / MaxHeat;
}

void UMainWeaponComponent::Fire()
{
	if (bIsOverheated)
	{
		StopFire();
		return;
	}

	FHitResult HitResult;
	if (TraceForHit(HitResult))
	{
		AActor* HitActor = HitResult.GetActor();
		APawn* OwnerPawn = Cast<APawn>(GetOwner());
		if (HitActor != nullptr && OwnerPawn != nullptr)
		{
			UGameplayStatics::ApplyDamage(HitActor, Damage, OwnerPawn->GetController(), GetOwner(), nullptr);
		}
	}

	AddHeat();
}

void UMainWeaponComponent::AddHeat()
{
	CurrentHeat = FMath::Clamp(CurrentHeat + HeatPerShot, ZeroThreshold, MaxHeat);
	OnHeatChanged.Broadcast(CurrentHeat, MaxHeat);

	if (CurrentHeat < MaxHeat)
	{
		return;
	}

	SetOverheated(true);
	StopFire();
}

void UMainWeaponComponent::UpdateCooling(float DeltaTime)
{
	if (CurrentHeat <= ZeroThreshold)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (World != nullptr && World->GetTimerManager().IsTimerActive(FireTimerHandle))
	{
		return;
	}

	CurrentHeat = FMath::Clamp(CurrentHeat - CoolingRate * DeltaTime, ZeroThreshold, MaxHeat);
	OnHeatChanged.Broadcast(CurrentHeat, MaxHeat);

	if (CurrentHeat > ZeroThreshold)
	{
		return;
	}

	SetOverheated(false);
}

void UMainWeaponComponent::SetOverheated(bool bNewOverheated)
{
	if (bIsOverheated == bNewOverheated)
	{
		return;
	}

	bIsOverheated = bNewOverheated;
	OnOverheatStateChanged.Broadcast(bIsOverheated);
}

bool UMainWeaponComponent::GetAimTarget(FVector& OutTarget) const
{
	UWorld* World = GetWorld();
	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!World || !OwnerPawn || TraceDistance <= 0.0f)
	{
		return false;
	}
	FVector RayStart;
	FVector RayDirection;
	const APlayerController* PlayerController = Cast<APlayerController>(OwnerPawn->GetController());
	if (PlayerController && PlayerController->IsLocalController())
	{
		int32 ViewportWidth = 0;
		int32 ViewportHeight = 0;
		PlayerController->GetViewportSize(ViewportWidth, ViewportHeight);
		if (ViewportWidth <= 0 || ViewportHeight <= 0
			|| !PlayerController->DeprojectScreenPositionToWorld(ViewportWidth * 0.5f,
				ViewportHeight * 0.5f, RayStart, RayDirection))
		{
			return false;
		}
	}
	else
	{
		// 플레이어 화면이 없는 Pawn은 기존 전방 조준을 사용합니다.
		RayStart = OwnerPawn->GetPawnViewLocation();
		RayDirection = OwnerPawn->GetActorForwardVector();
	}
	const FVector RayEnd = RayStart + RayDirection * TraceDistance;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
	FHitResult CameraHit;
	const bool bCameraHit = World->LineTraceSingleByChannel(CameraHit, RayStart, RayEnd,
		ECC_Visibility, QueryParams);
	OutTarget = bCameraHit ? CameraHit.ImpactPoint : RayEnd;
	return true;
}

bool UMainWeaponComponent::TraceForHit(FHitResult& OutHit) const
{
	UWorld* World = GetWorld();
	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	FVector AimTarget;
	if (!World || !OwnerPawn || !GetAimTarget(AimTarget))
	{
		return false;
	}
	const FVector TraceStart = GetMuzzleLocation();
	const FVector ShotDirection = (AimTarget - TraceStart).GetSafeNormal();
	if (ShotDirection.IsNearlyZero())
	{
		return false;
	}
	const FVector TraceEnd = TraceStart + ShotDirection * TraceDistance;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());

	// 총구가 벽을 뚫고 나갔을 때도 캐릭터와 총구 사이의 장애물에 막히게 합니다.
	const FVector GuardStart = OwnerPawn->GetActorLocation();
	bool bMuzzleBlocked = false;
	const bool bHit = TraceMuzzlePath(World, GuardStart, TraceStart, TraceEnd, QueryParams,
		OutHit, bMuzzleBlocked);
	DrawDebugLine(World, bMuzzleBlocked ? GuardStart : TraceStart,
		bHit ? OutHit.ImpactPoint : TraceEnd, bHit ? FColor::Red : FColor::Green, false, 1.0f, 0, 1.0f);

	if (bHit)
	{
		DrawDebugSphere(World, OutHit.ImpactPoint, 8.0f, 12, FColor::Red, false, 1.0f);
	}
	return bHit;
}

void UMainWeaponComponent::AttachToCharacterMesh(USkeletalMeshComponent* ParentMesh)
{
	if (ParentMesh == nullptr)
	{
		return;
	}

	if (WeaponMeshComponent != nullptr)
	{
		return;
	}

	AActor* OwnerActor = GetOwner();
	if (OwnerActor == nullptr)
	{
		return;
	}

	WeaponMeshComponent = NewObject<USkeletalMeshComponent>(OwnerActor);
	if (WeaponMeshComponent == nullptr)
	{
		return;
	}

	WeaponMeshComponent->SetSkeletalMesh(WeaponMesh);
	WeaponMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMeshComponent->SetupAttachment(ParentMesh, AttachSocketName);
	WeaponMeshComponent->RegisterComponent();
	WeaponMeshComponent->AttachToComponent(
		ParentMesh,
		FAttachmentTransformRules::SnapToTargetIncludingScale,
		AttachSocketName);
}

FVector UMainWeaponComponent::GetMuzzleLocation() const
{
	if (!WeaponMeshComponent || !WeaponMeshComponent->DoesSocketExist(MuzzleSocketName))
	{
		const APawn* OwnerPawn = Cast<APawn>(GetOwner());
		return OwnerPawn ? OwnerPawn->GetPawnViewLocation() : FVector::ZeroVector;
	}

	return WeaponMeshComponent->GetSocketLocation(MuzzleSocketName);
}
