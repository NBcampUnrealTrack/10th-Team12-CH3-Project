// VisionStealComponent.cpp
#include "VisionStealComponent.h"
#include "AIBaseCharacter.h"
#include "OptimusPrimeCharacter.h"
#include "HealthComponent.h"
#include "MainWeaponComponent.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/InputSettings.h"

UVisionStealComponent::UVisionStealComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void UVisionStealComponent::ToggleVisionSteal()
{
	if (bReconActive)
	{
		EndVisionSteal();
		return;
	}

	AOptimusPrimeCharacter* Player = Cast<AOptimusPrimeCharacter>(GetOwner());
	APlayerController* PC = Player ? Cast<APlayerController>(Player->GetController()) : nullptr;
	if (!PC || !PC->IsLocalController() || Player->IsCharacterDead())
	{
		return;
	}
	FVector ViewLocation;
	FRotator ViewRotation;
	PC->GetPlayerViewPoint(ViewLocation, ViewRotation);
	if (!StartVisionSteal(FindReconTarget(ViewLocation, ViewRotation.Vector()), PC))
	{
		ShowReconMessage(FString::Printf(TEXT("Recon: aim at a visible living enemy within %.0fm, then press Q."), ReconRange / 100.0f));
	}
}

AAIBaseCharacter* UVisionStealComponent::FindReconTarget(const FVector& ViewLocation, const FVector& ViewDirection) const
{
	// 적 BP의 Visibility 응답과 관계없이 가장 앞의 Pawn을 선택합니다.
	FCollisionQueryParams Params(SCENE_QUERY_STAT(VisionSteal), false, GetOwner());
	FCollisionObjectQueryParams PawnObjects(ECC_Pawn);
	FHitResult PawnHit;
	if (!GetWorld()->LineTraceSingleByObjectType(PawnHit, ViewLocation,
		ViewLocation + ViewDirection.GetSafeNormal() * ReconRange, PawnObjects, Params))
	{
		return nullptr;
	}
	AAIBaseCharacter* Target = Cast<AAIBaseCharacter>(PawnHit.GetActor());
	if (!Target) return nullptr;

	// Pawn만 찾으면 벽 너머도 맞으므로, 대상까지의 장애물 검사는 따로 수행합니다.
	Params.AddIgnoredActor(Target);
	FHitResult Obstacle;
	if (GetWorld()->LineTraceSingleByChannel(Obstacle, ViewLocation, PawnHit.ImpactPoint, ECC_Visibility, Params))
	{
		return nullptr;
	}
	return Target;
}

bool UVisionStealComponent::StartVisionSteal(AAIBaseCharacter* Target, APlayerController* PC)
{
	AOptimusPrimeCharacter* Player = Cast<AOptimusPrimeCharacter>(GetOwner());
	UHealthComponent* TargetHealth = IsValid(Target) ? Target->FindComponentByClass<UHealthComponent>() : nullptr;
	if (bReconActive || !Player || !PC || Player->IsCharacterDead() || !TargetHealth || !TargetHealth->IsAlive()
		|| Target->IsReconSuppressed())
	{
		return false;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Player;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ReconCamera = GetWorld()->SpawnActor<ACameraActor>(ACameraActor::StaticClass(), FTransform::Identity, SpawnParams);
	if (!ReconCamera)
	{
		return false;
	}
	ReconCamera->GetCameraComponent()->SetFieldOfView(ReconFOV);
	ReconCamera->GetCameraComponent()->bConstrainAspectRatio = false;
	ReconTarget = Target;
	ReconController = PC;
	PreviousViewTarget = PC->GetViewTarget();
	StartingYaw = Target->GetActorRotation().Yaw;
	SmoothedYaw = StartingYaw;
	ReconYawOffset = 0.0f;
	ReconPitchOffset = 0.0f;
	bReconActive = true;

	// 이동은 계속 허용하고, 내 몸은 이동 방향을 향하게 합니다. 사격만 중단합니다.
	UCharacterMovementComponent* Movement = Player->GetCharacterMovement();
	bPreviousOrientRotationToMovement = Movement->bOrientRotationToMovement;
	Movement->bOrientRotationToMovement = true;
	if (UMainWeaponComponent* Weapon = Player->FindComponentByClass<UMainWeaponComponent>())
	{
		Weapon->StopFire();
	}
	Player->StopAnimMontage();
	if (bFreezeTarget)
	{
		Target->SetReconSuppressed(true);
	}
	TargetHealth->OnDeath.AddUniqueDynamic(this, &UVisionStealComponent::HandleParticipantDeath);
	Player->FindComponentByClass<UHealthComponent>()->OnDeath.AddUniqueDynamic(this, &UVisionStealComponent::HandleParticipantDeath);
	Target->OnDestroyed.AddUniqueDynamic(this, &UVisionStealComponent::HandleTargetDestroyed);

	// 카메라 안쪽으로 보이는 대상 메시만 이 플레이어의 화면에서 숨깁니다.
	bAddedHiddenTarget = !PC->HiddenActors.Contains(Target);
	if (bAddedHiddenTarget)
	{
		PC->HiddenActors.Add(Target);
	}
	UpdateReconCamera();
	// 순간 전환으로 중간 경로의 벽을 통과하는 카메라 연출을 피합니다.
	PC->SetViewTargetWithBlend(ReconCamera, 0.0f);
	SetComponentTickEnabled(true);
	ShowReconMessage(TEXT("RECON | Mouse: look around | WASD: move YOUR body | Q: return"));
	UE_LOG(LogTemp, Display, TEXT("[Recon] Started: %s"), *Target->GetName());
	return true;
}

void UVisionStealComponent::UpdateReconLook(const FVector2D& LookInput)
{
	if (!bReconActive)
	{
		return;
	}
	// 월드 각도가 아닌 시작 방향에 대한 누적 각도를 제한합니다.
	ReconYawOffset = FMath::Clamp(ReconYawOffset + LookInput.X * ReconLookSensitivity, -ReconYawLimit, ReconYawLimit);
	// 기존 Look 입력과 동일한 피치 부호/레거시 배율을 사용합니다.
	const APlayerController* PC = ReconController.Get();
	const float PitchScale = PC && GetDefault<UInputSettings>()->bEnableLegacyInputScales ? PC->GetDeprecatedInputPitchScale() : 1.0f;
	ReconPitchOffset = FMath::Clamp(ReconPitchOffset + LookInput.Y * PitchScale * ReconLookSensitivity, -ReconPitchLimit, ReconPitchLimit);
	UpdateReconCamera();
}

FRotator UVisionStealComponent::GetReconViewRotation() const
{
	// 대상의 몸 방향은 TickComponent에서 SmoothedYaw로 부드럽게 따라갑니다.
	return FRotator(ReconPitchOffset, SmoothedYaw + ReconYawOffset, 0.0f);
}

void UVisionStealComponent::UpdateReconCamera()
{
	AAIBaseCharacter* Target = ReconTarget.Get();
	if (!Target || !IsValid(ReconCamera))
	{
		return;
	}
	FVector EyeLocation = Target->GetPawnViewLocation();
	// Paragon 캐릭터는 head 본을 사용합니다. 눈 높이를 쓰되 애니메이션의 머리 회전은 따라가지 않습니다.
	if (Target->GetMesh() && Target->GetMesh()->DoesSocketExist(TEXT("head")))
	{
		EyeLocation = Target->GetMesh()->GetSocketLocation(TEXT("head"));
	}
	ReconCamera->SetActorLocationAndRotation(EyeLocation, GetReconViewRotation());
}

void UVisionStealComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	const AOptimusPrimeCharacter* Player = Cast<AOptimusPrimeCharacter>(GetOwner());
	if (!ReconTarget.IsValid() || !ReconController.IsValid() || !IsValid(ReconCamera)
		|| !Player || Player->IsCharacterDead() || Player->GetController() != ReconController.Get())
	{
		EndVisionSteal();
		return;
	}
	// 숫자 Yaw 대신 회전끼리 보간해야 -180도와 180도 경계에서 가까운 쪽으로 돕니다.
	const FRotator TargetFacing(0.0f, ReconTarget->GetActorRotation().Yaw, 0.0f);
	SmoothedYaw = ReconYawFollowSpeed > 0.0f
		? FMath::RInterpConstantTo(FRotator(0.0f, SmoothedYaw, 0.0f), TargetFacing, DeltaTime, ReconYawFollowSpeed).Yaw
		: TargetFacing.Yaw;
	UpdateReconCamera();
}

void UVisionStealComponent::EndVisionSteal()
{
	if (!bReconActive)
	{
		return;
	}
	bReconActive = false;
	SetComponentTickEnabled(false);
	AAIBaseCharacter* Target = ReconTarget.Get();
	AOptimusPrimeCharacter* Player = Cast<AOptimusPrimeCharacter>(GetOwner());
	if (Target)
	{
		Target->OnDestroyed.RemoveDynamic(this, &UVisionStealComponent::HandleTargetDestroyed);
		if (UHealthComponent* Health = Target->FindComponentByClass<UHealthComponent>())
		{
			Health->OnDeath.RemoveDynamic(this, &UVisionStealComponent::HandleParticipantDeath);
		}
		Target->SetReconSuppressed(false);
	}
	if (APlayerController* PC = ReconController.Get())
	{
		if (bAddedHiddenTarget)
		{
			PC->HiddenActors.Remove(ReconTarget.Get(true));
		}
		PC->SetViewTargetWithBlend(PreviousViewTarget.IsValid() ? PreviousViewTarget.Get() : Player, 0.0f);
	}
	if (Player)
	{
		if (UHealthComponent* Health = Player->FindComponentByClass<UHealthComponent>())
		{
			Health->OnDeath.RemoveDynamic(this, &UVisionStealComponent::HandleParticipantDeath);
		}
		// 이동 모드를 저장/덮어쓰지 않으므로 정찰 중 점프·낙하·사망 상태가 유지됩니다.
		Player->GetCharacterMovement()->bOrientRotationToMovement = bPreviousOrientRotationToMovement;
	}
	if (IsValid(ReconCamera))
	{
		ReconCamera->Destroy();
	}
	ReconCamera = nullptr;
	ReconTarget.Reset();
	ReconController.Reset();
	PreviousViewTarget.Reset();
	bAddedHiddenTarget = false;
	ShowReconMessage(TEXT("Recon ended. Aim at an enemy and press Q to try again."));
	UE_LOG(LogTemp, Display, TEXT("[Recon] Ended"));
}

void UVisionStealComponent::HandleParticipantDeath(AActor* DeadOwner)
{
	EndVisionSteal();
}

void UVisionStealComponent::HandleTargetDestroyed(AActor* DestroyedActor)
{
	EndVisionSteal();
}

void UVisionStealComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	EndVisionSteal();
	Super::EndPlay(EndPlayReason);
}

void UVisionStealComponent::ShowReconMessage(const FString& Message) const
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(771045, 8.0f, FColor::Cyan, Message);
	}
}
