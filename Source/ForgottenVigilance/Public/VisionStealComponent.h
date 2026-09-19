// VisionStealComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Styling/SlateBrush.h"
#include "VisionStealComponent.generated.h"

class AAIBaseCharacter;
class ACameraActor;
class APlayerController;
class SWidget;
class USceneCaptureComponent2D;
class UTextureRenderTarget2D;

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
	// 화면 전체가 적 시점으로 바뀐 경우에만 참입니다. 화면 분할 중에는 평소처럼 조작합니다.
	bool IsFullScreenRecon() const { return bReconActive && !bActiveSplitView; }
	FRotator GetReconViewRotation() const;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	bool StartVisionSteal(AAIBaseCharacter* Target, APlayerController* PlayerController);
	AAIBaseCharacter* FindReconTarget(const FVector& ViewLocation, const FVector& ViewDirection) const;
	void UpdateReconCamera();
	void ShowReconMessage(const FString& Message) const;
	void ShowSplitView(AAIBaseCharacter* Target);
	void HideSplitView();

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
	// 화면이 대상의 몸 방향을 따라가는 속도, 초당 각도. 0이면 즉시 따라갑니다.
	UPROPERTY(EditAnywhere, Category="Recon", meta=(ClampMin="0.0"))
	float ReconYawFollowSpeed = 120.0f;
	// 비교용: 켜면 내 화면은 그대로 두고 오른쪽 위 작은 창에 적 시야를 띄웁니다.
	UPROPERTY(EditAnywhere, Category="Recon")
	bool bSplitView = true;
	// 작은 창이 화면에 표시되는 크기입니다.
	UPROPERTY(EditAnywhere, Category="Recon")
	FVector2D SplitViewSize = FVector2D(480.0, 270.0);
	// 적 시야를 촬영하는 해상도입니다. 클수록 선명하지만 무거워집니다.
	UPROPERTY(EditDefaultsOnly, Category="Recon")
	FIntPoint SplitViewResolution = FIntPoint(640, 360);

	UPROPERTY(Transient)
	TObjectPtr<USceneCaptureComponent2D> ReconCapture;
	UPROPERTY(Transient)
	TObjectPtr<UTextureRenderTarget2D> ReconRenderTarget;
	FSlateBrush ReconBrush;
	TSharedPtr<SWidget> ReconOverlay;
	bool bActiveSplitView = false;

	UPROPERTY(Transient)
	TObjectPtr<ACameraActor> ReconCamera;
	TWeakObjectPtr<AAIBaseCharacter> ReconTarget;
	TWeakObjectPtr<APlayerController> ReconController;
	TWeakObjectPtr<AActor> PreviousViewTarget;
	float StartingYaw = 0.0f;
	// 대상의 몸 방향을 부드럽게 따라가는 화면 기준 Yaw입니다.
	float SmoothedYaw = 0.0f;
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
