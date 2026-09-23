#include "AIRangeWeaponComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "TimerManager.h"

UAIRangeWeaponComponent::UAIRangeWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}

void UAIRangeWeaponComponent::FireGun()
{
	FHitResult HitResult;
	FVector ShotEnd = FVector::ZeroVector;

	const bool bHit = AITraceForHit(HitResult, ShotEnd);

	PlayShotEffects(ShotEnd);

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
	
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	if (HitActor != PlayerPawn)
	{
		return;
	}

	UGameplayStatics::ApplyDamage(
	    HitActor,
	    AIGunDamage,
	    OwnerPawn->GetController(),
	    GetOwner(),
	    nullptr);
}

bool UAIRangeWeaponComponent::AITraceForHit(FHitResult& OutHit, FVector& OutShotEnd) const
{

	UWorld* World = GetWorld();

	if (!World)
	{
		return false;
	}

	APawn* OwnerPawn = Cast<APawn>(GetOwner());

	if (!OwnerPawn)
	{
		return false;
	}

	const FVector TraceStart = OwnerPawn->GetActorLocation();

	const FVector TraceEnd =  TraceStart + OwnerPawn->GetActorForwardVector() * AIGunAttackRange;

	OutShotEnd = TraceEnd;

	FCollisionQueryParams QueryParams;

	QueryParams.AddIgnoredActor(GetOwner());

	const bool bHit =
	    World->LineTraceSingleByChannel(
	        OutHit,
	        TraceStart,
	        TraceEnd,
	        ECC_Pawn,
	        QueryParams);

	DrawDebugLine(
	    World,
	    TraceStart,
	    TraceEnd,
	    bHit ? FColor::Red : FColor::Green,
	    false,
	    1.0f,
	    0,
	    1.0f);

	if (bHit)
	{
		OutShotEnd = OutHit.ImpactPoint;
	}

	return bHit;
}

USkeletalMeshComponent* UAIRangeWeaponComponent::GetMuzzleMesh() const
{
	const ACharacter* OwnerCharacter =
	    Cast<ACharacter>(GetOwner());

	if (!OwnerCharacter)
	{
		return nullptr;
	}

	USkeletalMeshComponent* Mesh =
	    OwnerCharacter->GetMesh();

	if (!Mesh)
	{
		return nullptr;
	}

	if (!Mesh->DoesSocketExist(MuzzleSocketName))
	{
		UE_LOG(
		    LogTemp,
		    Warning,
		    TEXT("%s: Muzzle socket '%s' does not exist."),
		    *GetNameSafe(GetOwner()),
		    *MuzzleSocketName.ToString());

		return nullptr;
	}

	return Mesh;
}

FVector UAIRangeWeaponComponent::GetMuzzleLocation() const
{
	USkeletalMeshComponent* MuzzleMesh = GetMuzzleMesh();

	if (!MuzzleMesh)
	{
		const APawn* OwnerPawn = Cast<APawn>(GetOwner());

		return OwnerPawn
		           ? OwnerPawn->GetPawnViewLocation()
		           : FVector::ZeroVector;
	}

	return MuzzleMesh->GetSocketLocation(MuzzleSocketName);
}

void UAIRangeWeaponComponent::PlayMuzzleFlash()
{
	StopMuzzleEffect();

	USkeletalMeshComponent* MuzzleMesh = GetMuzzleMesh();

	if (!MuzzleMesh)
	{
		return;
	}

	ActiveMuzzleComponent =
	    SpawnEffectAttached(
	        MuzzleEffect,
	        MuzzleMesh,
	        MuzzleSocketName);

	if (!ActiveMuzzleComponent)
	{
		return;
	}

	GetWorld()->GetTimerManager().SetTimer(
	    MuzzleStopTimerHandle,
	    this,
	    &UAIRangeWeaponComponent::StopMuzzleEffect,
	    MuzzleEffectLifetime,
	    false);
}

void UAIRangeWeaponComponent::StopMuzzleEffect()
{
	UWorld* World = GetWorld();

	if (World)
	{
		World->GetTimerManager().ClearTimer(
		    MuzzleStopTimerHandle);
	}

	if (!ActiveMuzzleComponent)
	{
		return;
	}

	ActiveMuzzleComponent->Deactivate();
	ActiveMuzzleComponent = nullptr;
}

void UAIRangeWeaponComponent::PlayTracer(
    const FVector& TracerStart,
    const FVector& TracerEnd) const
{
	UFXSystemComponent* TracerComponent =
	    SpawnEffectAtLocation(
	        TracerEffect,
	        TracerStart,
	        (TracerEnd - TracerStart).Rotation());

	if (!TracerComponent)
	{
		return;
	}

	UNiagaraComponent* NiagaraTracer =
	    Cast<UNiagaraComponent>(TracerComponent);

	if (!NiagaraTracer)
	{
		TracerComponent->SetVectorParameter(
		    TracerEndParameterName,
		    TracerEnd);

		return;
	}

	NiagaraTracer->SetVariableVec3(
	    TracerEndParameterName,
	    TracerEnd);
}

void UAIRangeWeaponComponent::PlayShotEffects(
    const FVector& ShotEnd)
{
	PlayMuzzleFlash();

	PlayTracer(
	    GetMuzzleLocation(),
	    ShotEnd);
}

UFXSystemComponent* UAIRangeWeaponComponent::SpawnEffectAttached(
    UFXSystemAsset* Effect,
    USceneComponent* Parent,
    FName SocketName) const
{
	if (!Effect || !Parent)
	{
		return nullptr;
	}

	if (UNiagaraSystem* NiagaraEffect =
	        Cast<UNiagaraSystem>(Effect))
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

	UParticleSystem* CascadeEffect =
	    Cast<UParticleSystem>(Effect);

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


UFXSystemComponent* UAIRangeWeaponComponent::SpawnEffectAtLocation(
    UFXSystemAsset* Effect,
    const FVector& Location,
    const FRotator& Rotation) const
{
	if (!Effect)
	{
		return nullptr;
	}

	if (UNiagaraSystem* NiagaraEffect =
	        Cast<UNiagaraSystem>(Effect))
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

	UParticleSystem* CascadeEffect =
	    Cast<UParticleSystem>(Effect);

	if (!CascadeEffect)
	{
		return nullptr;
	}

	return UGameplayStatics::SpawnEmitterAtLocation(
	    this,
	    CascadeEffect,
	    Location,
	    Rotation,
	    true);
}
