#include "MainWeaponComponent.h"

#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "GameFramework/Character.h"

namespace
{
constexpr float DefaultDamage = 50.0f;
constexpr float DefaultFireInterval = 0.1f;
constexpr float DefaultTraceDistance = 10000.0f;
constexpr float DefaultMaxHeat = 100.0f;
constexpr float DefaultHeatPerShot = 0.0f;
constexpr float DefaultCoolingRate = 25.0f;
constexpr float ZeroThreshold = 0.0f;
const FName DefaultAttachSocketName(TEXT("WeaponSocket"));
const FName DefaultMuzzleSocketName(TEXT("Muzzle"));
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

bool UMainWeaponComponent::TraceForHit(FHitResult& OutHit) const
{
    UWorld* World = GetWorld();
    if (World == nullptr)
    {
       return false;
    }

    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (OwnerPawn == nullptr)
    {
       return false;
    }

    const FVector TraceStart = OwnerPawn->GetActorLocation();
    const FVector TraceEnd = TraceStart + OwnerPawn->GetActorForwardVector() * TraceDistance;

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
	
	//디버그용 start
	const bool bHit = World->LineTraceSingleByChannel(OutHit, TraceStart, TraceEnd, ECC_Visibility, QueryParams);

	DrawDebugLine(World, TraceStart, bHit ? OutHit.ImpactPoint : TraceEnd, bHit ? FColor::Red : FColor::Green, false, 1.0f, 0, 1.0f);

	if (bHit)
	{
		DrawDebugSphere(World, OutHit.ImpactPoint, 8.0f, 12, FColor::Red, false, 1.0f);
	}
	//end

    return World->LineTraceSingleByChannel(OutHit, TraceStart, TraceEnd, ECC_Visibility, QueryParams);
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
	if (WeaponMeshComponent == nullptr)
	{
		return FVector::ZeroVector;
	}

	return WeaponMeshComponent->GetSocketLocation(MuzzleSocketName);
}
