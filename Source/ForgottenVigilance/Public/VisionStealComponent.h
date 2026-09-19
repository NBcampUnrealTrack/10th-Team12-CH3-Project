// VisionStealComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VisionStealComponent.generated.h"

class AAIBaseCharacter;
class ACameraActor;
class APlayerController;

// Q로 정찰 감각을 검증하는 로컬 시제품. 입력 에셋이나 빙의를 변경하지 않습니다.
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FORGOTTENVIGILANCE_API UVisionStealComponent : public UActorComponent
{

	GENERATED_BODY()

public:
	UVisionStealComponent();
	void ToggleVisionSteal();
	void UpdateReconLook(const FVector2D& LookInput);
	void EndVisionSteal();
	bool IsReconActive() const { return bReconActive; }
	FRotator GetReconViewRotation() const;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	bool StartVisionSteal(AAIBaseCharacter* Target, APlayerController* PlayerController);
	AAIBaseCharacter* FindReconTarget(const FVector& ViewLocation, const FVector& ViewDirection) const;
	void UpdateReconCamera();
	void ShowReconMessage(const FString& Message) const;

	UFUNCTION()
	void HandleParticipantDeath(AActor* DeadOwner);
	UFUNCTION()
	void HandleTargetDestroyed(AActor* DestroyedActor);

	UPROPERTY(EditDefaultsOnly, Category="Recon", meta=(ClampMin="1.0", Units="cm"))
	float ReconRange = 3000.0f;
	UPROPERTY(EditDefaultsOnly, Category="Recon", meta=(ClampMin="0.0", ClampMax="180.0"))
	float ReconYawLimit = 45.0f;
	UPROPERTY(EditDefaultsOnly, Category="Recon", meta=(ClampMin="0.0", ClampMax="89.0"))
	float ReconPitchLimit = 60.0f;
	UPROPERTY(EditDefaultsOnly, Category="Recon", meta=(ClampMin="0.01"))
	float ReconLookSensitivity = 1.0f;
	UPROPERTY(EditDefaultsOnly, Category="Recon", meta=(ClampMin="30.0", ClampMax="120.0"))
	float ReconFOV = 90.0f;
	// 비교용: 끄면 정찰 중에도 대상 AI가 계속 움직이고 공격합니다.
	UPROPERTY(EditAnywhere, Category="Recon")
	bool bFreezeTarget = true;

	UPROPERTY(Transient)
	TObjectPtr<ACameraActor> ReconCamera;
	TWeakObjectPtr<AAIBaseCharacter> ReconTarget;
	TWeakObjectPtr<APlayerController> ReconController;
	TWeakObjectPtr<AActor> PreviousViewTarget;
	float StartingYaw = 0.0f;
	float ReconYawOffset = 0.0f;
	float ReconPitchOffset = 0.0f;
	bool bPreviousOrientRotationToMovement = false;
	bool bReconActive = false;
	bool bAddedHiddenTarget = false;

#if WITH_DEV_AUTOMATION_TESTS
	friend class FVisionStealLifecycleTest;
	friend class FVisionStealTargetingTest;
#endif
};
