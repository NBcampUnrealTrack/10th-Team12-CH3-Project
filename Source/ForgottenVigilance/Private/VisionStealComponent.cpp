
#include "VisionStealComponent.h"
#include "AIBaseCharacter.h"
#include "Camera/CameraActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "HealthComponent.h"

void UVisionStealComponent::ToggleVisionSteal()
{
	if (bVisionStealActive)
	{
		if (bForcedVisionSteal)
		{
			return;
		}

		EndVisionSteal();
		return;
	}

	if (CurrentCharges <= 0)
	{
		return;
	}

	AAIBaseCharacter* TargetAIBaseCharacter = FindTargetByCrosshair();

	if (!TargetAIBaseCharacter)
	{
		return;
	}

	BeginVisionSteal(TargetAIBaseCharacter, VisionStealDuration, true);
}

bool UVisionStealComponent::StartVisionSteal(AActor* Target)
{
	return BeginVisionSteal(Target, VisionStealDuration, true);
}

void UVisionStealComponent::ForceVisionSteal(AActor* Target, float Duration)
{
	if (bVisionStealActive)
	{
		EndVisionSteal();
	}

	if (!BeginVisionSteal(Target, Duration, false))
	{
		return;
	}

	bForcedVisionSteal = true;
}

bool UVisionStealComponent::BeginVisionSteal(AActor* Target, float Duration, bool bConsumeCharge)
{
	if (!IsValid(Target) || bVisionStealActive)
	{
		return false;
	}

	if (bConsumeCharge && CurrentCharges <= 0)
	{
		return false;
	}

	const APawn* OwnerPawn = Cast<APawn>(GetOwner());

	if (!OwnerPawn)
	{
		return false;
	}

	APlayerController* PlayerController = Cast<APlayerController>(OwnerPawn->GetController());

	if (!PlayerController)
	{
		return false;
	}

	AActor* ViewTarget = Target;

	if (AAIBaseCharacter* TargetCharacter = Cast<AAIBaseCharacter>(Target))
	{
		if (ACameraActor* CameraActor = SpawnCameraActor(TargetCharacter))
		{
			SpawnedVisionCameraActor = CameraActor;
			ViewTarget = CameraActor;
		}
	}

	PlayerController->SetViewTargetWithBlend(ViewTarget, ViewTargetBlendTime);

	bVisionStealActive = true;
	VisionStealTarget = Target;

	UHealthComponent* TargetHealth = Target->FindComponentByClass<UHealthComponent>();

	if (TargetHealth)
	{
		TargetHealth->OnDeath.AddDynamic(this, &UVisionStealComponent::HandleTargetDeath);
	}

	if (bConsumeCharge)
	{
		CurrentCharges -= 1;
		OnVisionStealChargeChanged.Broadcast(CurrentCharges, MaxCharges);
	}

	OnVisionStealStateChanged.Broadcast(true);

	if (Duration <= 0.0f)
	{
		return true;
	}

	GetWorld()->GetTimerManager().SetTimer(
		VisionStealTimerHandle,
		this,
		&UVisionStealComponent::HandleVisionStealTimeout,
		Duration,
		false);

	return true;
}

void UVisionStealComponent::HandleVisionStealTimeout()
{
	bForcedVisionSteal = false;
	EndVisionSteal();
}

ACameraActor* UVisionStealComponent::SpawnCameraActor(AAIBaseCharacter* Target)
{
	if (!IsValid(Target))
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnCameraActor: invalid target"));
		return nullptr;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnCameraActor: world not found"));
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Target;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ACameraActor* CameraActor = World->SpawnActor<ACameraActor>(
	    ACameraActor::StaticClass(), Target->GetActorTransform(), SpawnParams);
	if (!CameraActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnCameraActor: spawn failed"));
		return nullptr;
	}

	USkeletalMeshComponent* TargetMesh = Target->GetMesh();
	USceneComponent* AttachParent = Target->GetRootComponent();
	FName AttachSocket = NAME_None;

	if (!CameraAttachSocketName.IsNone())
	{
		if (TargetMesh && TargetMesh->DoesSocketExist(CameraAttachSocketName))
		{
			AttachParent = TargetMesh;
			AttachSocket = CameraAttachSocketName;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("SpawnCameraActor: socket %s not found on %s, falling back to root"),
			    *CameraAttachSocketName.ToString(), *GetNameSafe(Target));
		}
	}

	CameraActor->AttachToComponent(AttachParent, FAttachmentTransformRules::SnapToTargetNotIncludingScale, AttachSocket);
	CameraActor->SetActorRelativeLocation(CameraRelativeLocation);
	CameraActor->SetActorRelativeRotation(CameraRelativeRotation);

	UE_LOG(LogTemp, Warning, TEXT("SpawnCameraActor: attached to %s socket %s"),
	    *GetNameSafe(Target), *AttachSocket.ToString());

	return CameraActor;
}

void UVisionStealComponent::EndVisionSteal()
{
	if (!bVisionStealActive)
	{
		return;
	}

	UWorld* World = GetWorld();

	if (World)
	{
		World->GetTimerManager().ClearTimer(VisionStealTimerHandle);
	}

	APawn* OwnerPawn = Cast<APawn>(GetOwner());

	if (OwnerPawn)
	{
		APlayerController* PlayerController = Cast<APlayerController>(OwnerPawn->GetController());

		if (PlayerController)
		{
			PlayerController->SetViewTargetWithBlend(OwnerPawn, ViewTargetBlendTime);
		}
	}

	if (IsValid(SpawnedVisionCameraActor))
	{
		SpawnedVisionCameraActor->Destroy();
	}
	
	if (IsValid(VisionStealTarget))
	{
		UHealthComponent* TargetHealth = VisionStealTarget->FindComponentByClass<UHealthComponent>();

		if (TargetHealth)
		{
			TargetHealth->OnDeath.RemoveDynamic(this, &UVisionStealComponent::HandleTargetDeath);
		}
	}

	VisionStealTarget = nullptr;

	SpawnedVisionCameraActor = nullptr;
	bVisionStealActive = false;
	bForcedVisionSteal = false;

	OnVisionStealStateChanged.Broadcast(false);
}

UVisionStealComponent::UVisionStealComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UVisionStealComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentCharges = MaxCharges;
	OnVisionStealChargeChanged.Broadcast(CurrentCharges, MaxCharges);
	OnVisionStealStateChanged.Broadcast(false);
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

	if (!bVisionStealActive)
	{
		return;
	}

	if (IsValid(SpawnedVisionCameraActor))
	{
		return;
	}

	bForcedVisionSteal = false;
	EndVisionSteal();
}

void UVisionStealComponent::HandleTargetDeath(AActor* DeadOwner)
{
	bForcedVisionSteal = false;
	EndVisionSteal();
}
