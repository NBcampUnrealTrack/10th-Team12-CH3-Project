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
#include "Perception/AISense_Hearing.h"
#include "Sound/SoundBase.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"

namespace
{
constexpr float DefaultDamage = 35.0f;
constexpr float DefaultFireInterval = 0.2f;
constexpr float DefaultTraceDistance = 10000.0f;
constexpr float DefaultMaxHeat = 100.0f;
constexpr float DefaultHeatPerShot = 3.5f;
constexpr float DefaultCoolingRate = 30.0f;
constexpr float MainWeaponZeroThreshold = 0.0f;
const FName DefaultAttachSocketName(TEXT("weapon"));
const FName DefaultRightMuzzleSocket(TEXT("Muzzle_01"));
const FName DefaultLeftMuzzleSocket(TEXT("Muzzle_02"));
const FName DefaultTracerEndParameter(TEXT("BeamEnd"));
constexpr int32 FirstMuzzleIndex = 0;
constexpr int32 MuzzleIndexStep = 1;
constexpr float DefaultNoiseLoudness = 1.0f;
constexpr float DefaultNoiseRange = 3000.0f;
constexpr float DefaultMuzzleEffectLifetime = 0.08f;

bool TraceMuzzlePath(UWorld* World, const FVector& GuardStart, const FVector& MuzzleStart,
	const FVector& TraceEnd, const FCollisionQueryParams& QueryParams, FHitResult& OutHit, bool& bMuzzleBlocked)
{
	bMuzzleBlocked = World->LineTraceSingleByChannel(OutHit, GuardStart, MuzzleStart,
		ECC_Visibility, QueryParams);
	return bMuzzleBlocked || World->LineTraceSingleByChannel(OutHit, MuzzleStart,
		TraceEnd, ECC_Pawn, QueryParams);
}
}

UMainWeaponComponent::UMainWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	WeaponMesh = nullptr;
	WeaponMeshComponent = nullptr;
	AttachSocketName = DefaultAttachSocketName;

	Damage = DefaultDamage;
	FireInterval = DefaultFireInterval;
	TraceDistance = DefaultTraceDistance;
	MaxHeat = DefaultMaxHeat;
	HeatPerShot = DefaultHeatPerShot;
	CoolingRate = DefaultCoolingRate;
	CurrentHeat = MainWeaponZeroThreshold;
	bIsOverheated = false;
	
	NoiseLoudness = DefaultNoiseLoudness;
	NoiseRange = DefaultNoiseRange;
	
	MuzzleSocketNames.Add(DefaultRightMuzzleSocket);
	MuzzleSocketNames.Add(DefaultLeftMuzzleSocket);
	bRandomizeMuzzle = true;
	CurrentMuzzleIndex = FirstMuzzleIndex;
	MuzzleEffect = nullptr;
	ImpactWorldEffect = nullptr;
	ImpactCharacterEffect = nullptr;
	TracerEffect = nullptr;
	TracerEndParameterName = DefaultTracerEndParameter;
	MuzzleEffectScale = FVector::OneVector;
	MuzzleEffectLifetime = DefaultMuzzleEffectLifetime;
	ActiveMuzzleComponent = nullptr;
}

void UMainWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentHeat = MainWeaponZeroThreshold;
	bIsOverheated = false;
	OnHeatChanged.Broadcast(CurrentHeat, MaxHeat);

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter == nullptr)
	{
		return;
	}

	AttachToCharacterMesh(OwnerCharacter->GetMesh());

	for (const FName& SocketName : MuzzleSocketNames)
	{
		if (GetMuzzleMesh(SocketName))
		{
			continue;
		}

		UE_LOG(LogTemp, Warning, TEXT("%s: muzzle socket '%s' is unavailable."),
			*GetNameSafe(GetOwner()), *SocketName.ToString());
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
	StopMuzzleEffect();
}

bool UMainWeaponComponent::IsOverheated() const
{
	return bIsOverheated;
}

float UMainWeaponComponent::GetHeatRatio() const
{
	if (MaxHeat <= MainWeaponZeroThreshold)
	{
		return MainWeaponZeroThreshold;
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
	SelectMuzzleForShot();
	OnShotFired.Broadcast(CurrentMuzzleIndex);
	
	if (FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			FireSound,
			GetMuzzleLocation(),
			1.0f,
			1.0f,
			0.0f,
			SoundAttenuation);
	}

	APawn* NoiseInstigator = Cast<APawn>(GetOwner());

	if (NoiseInstigator)
	{
		UAISense_Hearing::ReportNoiseEvent(
			GetWorld(),
			GetMuzzleLocation(),
			NoiseLoudness,
			NoiseInstigator,
			NoiseRange);
	}

	FHitResult HitResult;
	FVector ShotEnd = FVector::ZeroVector;
	const bool bHit = TraceForHit(HitResult, ShotEnd);

	PlayShotEffects(bHit, HitResult, ShotEnd);
	AddHeat();

	if (!bHit)
	{
		return;
	}

	AActor* HitActor = HitResult.GetActor();
	APawn* OwnerPawn = Cast<APawn>(GetOwner());

	if (!HitActor || !OwnerPawn)
	{
		return;
	}

	UGameplayStatics::ApplyDamage(HitActor, Damage, OwnerPawn->GetController(), GetOwner(), nullptr);
}

void UMainWeaponComponent::AddHeat()
{
	CurrentHeat = FMath::Clamp(CurrentHeat + HeatPerShot, MainWeaponZeroThreshold, MaxHeat);
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
	if (CurrentHeat <= MainWeaponZeroThreshold)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (World != nullptr && World->GetTimerManager().IsTimerActive(FireTimerHandle))
	{
		return;
	}

	CurrentHeat = FMath::Clamp(CurrentHeat - CoolingRate * DeltaTime, MainWeaponZeroThreshold, MaxHeat);
	OnHeatChanged.Broadcast(CurrentHeat, MaxHeat);

	if (CurrentHeat > MainWeaponZeroThreshold)
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

	if (bIsOverheated && OverheatSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, OverheatSound, GetMuzzleLocation(), 1.0f, 1.0f, 0.0f, SoundAttenuation);
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

bool UMainWeaponComponent::TraceForHit(FHitResult& OutHit, FVector& OutShotEnd) const
{
	UWorld* World = GetWorld();
	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	FVector AimTarget;
	OutShotEnd = GetMuzzleLocation();

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

	const FVector GuardStart = OwnerPawn->GetActorLocation();
	bool bMuzzleBlocked = false;
	const bool bHit = TraceMuzzlePath(World, GuardStart, TraceStart, TraceEnd, QueryParams, OutHit, bMuzzleBlocked);

	OutShotEnd = bHit ? OutHit.ImpactPoint : TraceEnd;

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
	const FName SocketName = GetCurrentMuzzleSocket();
	USkeletalMeshComponent* MuzzleMesh = GetMuzzleMesh(SocketName);

	if (!MuzzleMesh)
	{
		const APawn* OwnerPawn = Cast<APawn>(GetOwner());
		return OwnerPawn ? OwnerPawn->GetPawnViewLocation() : FVector::ZeroVector;
	}

	return MuzzleMesh->GetSocketLocation(SocketName);
}

FName UMainWeaponComponent::GetCurrentMuzzleSocket() const
{
	if (!MuzzleSocketNames.IsValidIndex(CurrentMuzzleIndex))
	{
		return NAME_None;
	}

	return MuzzleSocketNames[CurrentMuzzleIndex];
}

void UMainWeaponComponent::SelectMuzzleForShot()
{
	const int32 MuzzleCount = MuzzleSocketNames.Num();

	if (MuzzleCount <= MuzzleIndexStep)
	{
		CurrentMuzzleIndex = FirstMuzzleIndex;
		return;
	}

	if (bRandomizeMuzzle)
	{
		CurrentMuzzleIndex = FMath::RandRange(FirstMuzzleIndex, MuzzleCount - MuzzleIndexStep);
		return;
	}

	CurrentMuzzleIndex = (CurrentMuzzleIndex + MuzzleIndexStep) % MuzzleCount;
}

USkeletalMeshComponent* UMainWeaponComponent::GetMuzzleMesh(FName SocketName) const
{
	if (SocketName.IsNone())
	{
		return nullptr;
	}

	if (WeaponMeshComponent && WeaponMeshComponent->DoesSocketExist(SocketName))
	{
		return WeaponMeshComponent;
	}

	const ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());

	if (!OwnerCharacter || !OwnerCharacter->GetMesh())
	{
		return nullptr;
	}

	if (!OwnerCharacter->GetMesh()->DoesSocketExist(SocketName))
	{
		return nullptr;
	}

	return OwnerCharacter->GetMesh();
}

void UMainWeaponComponent::PlayShotEffects(bool bHit, const FHitResult& HitResult, const FVector& ShotEnd)
{
	PlayMuzzleFlash();
	PlayTracer(GetMuzzleLocation(), ShotEnd);

	if (!bHit)
	{
		return;
	}

	PlayImpact(HitResult);
}

void UMainWeaponComponent::PlayMuzzleFlash()
{
	StopMuzzleEffect();

	const FName SocketName = GetCurrentMuzzleSocket();
	ActiveMuzzleComponent = SpawnEffectAttached(MuzzleEffect, GetMuzzleMesh(SocketName), SocketName);

	if (!ActiveMuzzleComponent)
	{
		return;
	}

	GetWorld()->GetTimerManager().SetTimer(
		MuzzleStopTimerHandle,
		this,
		&UMainWeaponComponent::StopMuzzleEffect,
		MuzzleEffectLifetime,
		false);
}

void UMainWeaponComponent::StopMuzzleEffect()
{
	UWorld* World = GetWorld();

	if (World)
	{
		World->GetTimerManager().ClearTimer(MuzzleStopTimerHandle);
	}

	if (!ActiveMuzzleComponent)
	{
		return;
	}

	ActiveMuzzleComponent->Deactivate();
	ActiveMuzzleComponent = nullptr;
}

void UMainWeaponComponent::PlayTracer(const FVector& TracerStart, const FVector& TracerEnd) const
{
	UFXSystemComponent* TracerComponent = SpawnEffectAtLocation(
		TracerEffect,
		TracerStart,
		(TracerEnd - TracerStart).Rotation());

	if (!TracerComponent)
	{
		return;
	}

	TracerComponent->SetVectorParameter(TracerEndParameterName, TracerEnd);
}

void UMainWeaponComponent::PlayImpact(const FHitResult& HitResult) const
{
	const bool bHitCharacter = Cast<APawn>(HitResult.GetActor()) != nullptr;
	UFXSystemAsset* ImpactEffect = bHitCharacter ? ImpactCharacterEffect : ImpactWorldEffect;

	SpawnEffectAtLocation(ImpactEffect, HitResult.ImpactPoint, HitResult.ImpactNormal.Rotation());
}

UFXSystemComponent* UMainWeaponComponent::SpawnEffectAttached(
	UFXSystemAsset* Effect,
	USceneComponent* Parent,
	FName SocketName) const
{
	if (!Effect || !Parent)
	{
		return nullptr;
	}

	if (UNiagaraSystem* NiagaraEffect = Cast<UNiagaraSystem>(Effect))
	{
		return UNiagaraFunctionLibrary::SpawnSystemAttached(
			NiagaraEffect,
			Parent,
			SocketName,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			MuzzleEffectScale,
			EAttachLocation::SnapToTarget,
			true,
			ENCPoolMethod::AutoRelease);
	}

	UParticleSystem* CascadeEffect = Cast<UParticleSystem>(Effect);

	if (!CascadeEffect)
	{
		return nullptr;
	}

	return UGameplayStatics::SpawnEmitterAttached(
		CascadeEffect,
		Parent,
		SocketName,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		MuzzleEffectScale,
		EAttachLocation::SnapToTarget,
		true);
}

UFXSystemComponent* UMainWeaponComponent::SpawnEffectAtLocation(
	UFXSystemAsset* Effect,
	const FVector& Location,
	const FRotator& Rotation) const
{
	if (!Effect)
	{
		return nullptr;
	}

	if (UNiagaraSystem* NiagaraEffect = Cast<UNiagaraSystem>(Effect))
	{
		return UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this,
			NiagaraEffect,
			Location,
			Rotation,
			FVector::OneVector,
			true,
			true,
			ENCPoolMethod::AutoRelease);
	}

	UParticleSystem* CascadeEffect = Cast<UParticleSystem>(Effect);

	if (!CascadeEffect)
	{
		return nullptr;
	}

	return UGameplayStatics::SpawnEmitterAtLocation(this, CascadeEffect, Location, Rotation, true);
}
