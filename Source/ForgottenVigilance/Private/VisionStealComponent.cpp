
#include "VisionStealComponent.h"
#include "AIBaseCharacter.h"
#include "DrawDebugHelpers.h"

void UVisionStealComponent::ToggleVisionSteal()
{
	if (!bVisionStealActive)
	{
		AAIBaseCharacter* TargetAIBaseCharacter = FindTargetByCrosshair();
		StartVisionSteal(TargetAIBaseCharacter);
	}
	else
	{
		EndVisionSteal();
	}
}

bool UVisionStealComponent::StartVisionSteal(AActor* Target)
{
	if (!IsValid(Target))
	{
		UE_LOG(LogTemp, Warning, TEXT("StartVisionSteal: invalid target"));
		return false;
	}

	if (bVisionStealActive)
	{
		UE_LOG(LogTemp, Warning, TEXT("StartVisionSteal: already active"));
		return false;
	}

	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("StartVisionSteal: owner is not a pawn"));
		return false;
	}

	APlayerController* PlayerController = Cast<APlayerController>(OwnerPawn->GetController());
	if (!PlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("StartVisionSteal: player controller not found"));
		return false;
	}

	PlayerController->SetViewTargetWithBlend(Target, ViewTargetBlendTime);

	UE_LOG(LogTemp, Warning, TEXT("StartVisionSteal: view switched to %s"), *GetNameSafe(Target));

	bVisionStealActive = true;
	return true;
}

void UVisionStealComponent::EndVisionSteal()
{
	if (!bVisionStealActive)
	{
		UE_LOG(LogTemp, Warning, TEXT("EndVisionSteal: not active"));
		return;
	}

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
	{
		return;
	}
	
	APlayerController* PlayerController = Cast<APlayerController>(OwnerPawn->GetController());
	if (!PlayerController)
	{
		return;
	}

	PlayerController->SetViewTargetWithBlend(OwnerPawn, ViewTargetBlendTime);
	
	bVisionStealActive = false;
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

