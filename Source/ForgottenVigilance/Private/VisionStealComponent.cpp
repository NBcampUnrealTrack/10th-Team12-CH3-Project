
#include "VisionStealComponent.h"
#include "AIBaseCharacter.h"
#include "DrawDebugHelpers.h"

void UVisionStealComponent::ToggleVisionSteal()
{
	FindTargetByCrosshair();
}

UVisionStealComponent::UVisionStealComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UVisionStealComponent::BeginPlay()
{
	Super::BeginPlay();
}

AAIBaseCharacter* UVisionStealComponent::FindTargetByCrosshair() const
{
	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("FindTargetByCrosshair: owner is not a pawn"));
		return nullptr;
	}

	APlayerController* PlayerController = Cast<APlayerController>(OwnerPawn->GetController());
	if (!PlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("FindTargetByCrosshair: player controller not found"));
		return nullptr;
	}
	FVector ViewLocation;
	FRotator ViewRotation;

	PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);

	const FVector EndPointLocation = ViewLocation + ViewRotation.Vector() * MaxTargetDistance;

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerPawn);

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

	const UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	const bool bIsHit = World->LineTraceSingleByObjectType(
	    HitResult, ViewLocation, EndPointLocation, ObjectQueryParams, QueryParams);

	DrawDebugLine(World, ViewLocation, EndPointLocation, FColor::Green, false, 3.0f, 0, 1.0f);

	if (!bIsHit)
	{
		UE_LOG(LogTemp, Warning, TEXT("FindTargetByCrosshair: nothing hit"));
		return nullptr;
	}

	DrawDebugPoint(World, HitResult.ImpactPoint, 15.0f, FColor::Red, false, 3.0f);

	if (AAIBaseCharacter* TargetEnemy = Cast<AAIBaseCharacter>(HitResult.GetActor()))
	{
		QueryParams.AddIgnoredActor(TargetEnemy);
		if (FHitResult ObstacleHit;
		    World->LineTraceSingleByChannel(ObstacleHit, ViewLocation, HitResult.ImpactPoint, ECC_Visibility, QueryParams))
		{

			UE_LOG(LogTemp, Warning, TEXT("FindTargetByCrosshair: target blocked %s"), *GetNameSafe(ObstacleHit.GetActor()));
			return nullptr;
		}

		UE_LOG(LogTemp, Warning, TEXT("Target found: %s"), *TargetEnemy->GetName());
		return TargetEnemy;
	}
	UE_LOG(LogTemp, Warning, TEXT("FindTargetByCrosshair: hit %s, not an enemy"), *GetNameSafe(HitResult.GetActor()));

	return nullptr;
}

void UVisionStealComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

