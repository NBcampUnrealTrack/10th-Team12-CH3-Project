#include "AIRangeWeaponComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"


UAIRangeWeaponComponent::UAIRangeWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}

void UAIRangeWeaponComponent::FireGun()
{
	FHitResult HitResult;

	if (!AITraceForHit(HitResult))
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

bool UAIRangeWeaponComponent::AITraceForHit(FHitResult& OutHit) const
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

	return bHit;
}



