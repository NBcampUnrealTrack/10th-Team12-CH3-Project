
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VisionStealComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVisionStealChargeChanged, int32, CurrentCharges, int32, MaxCharges);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVisionStealStateChanged, bool, bIsActive);

class AAIBaseCharacter;
class ACameraActor;
class UForgottenHUDWidget;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class FORGOTTENVIGILANCE_API UVisionStealComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void ForceVisionSteal(AActor* Target, float Duration);
	int32 GetCurrentCharges() const { return CurrentCharges; }
	int32 GetMaxCharges() const { return MaxCharges; }

	UFUNCTION()
	void ToggleVisionSteal();
	UFUNCTION(BlueprintCallable)
	bool StartVisionSteal(AActor* Target);
	UFUNCTION(BlueprintCallable)
	void EndVisionSteal();

	bool IsVisionStealActive() const { return bVisionStealActive; }

	UVisionStealComponent();
	
	UPROPERTY(BlueprintAssignable, Category = "VisionSteal")
	FOnVisionStealChargeChanged OnVisionStealChargeChanged;

	UPROPERTY(BlueprintAssignable, Category = "VisionSteal")
	FOnVisionStealStateChanged OnVisionStealStateChanged;

protected:
	virtual void BeginPlay() override;

private:
	ACameraActor* SpawnCameraActor(AAIBaseCharacter* Target);

	AAIBaseCharacter* FindTargetByCrosshair() const;

	UPROPERTY(EditAnywhere, Category = "VisionSteal")
	float MaxTargetDistance = 3000.0f;
	UPROPERTY(EditAnywhere, Category = "VisionSteal")
	float ViewTargetBlendTime = 0.05f;
	UPROPERTY(VisibleAnywhere, Category = "VisionSteal")
	bool bVisionStealActive = false;

	UPROPERTY(VisibleAnywhere, Category = "VisionSteal|Camera")
	TObjectPtr<ACameraActor> SpawnedVisionCameraActor;
	UPROPERTY(EditAnywhere, Category = "VisionSteal|Camera")
	FName CameraAttachSocketName = NAME_None;
	UPROPERTY(EditAnywhere, Category = "VisionSteal|Camera")
	FVector CameraRelativeLocation = FVector(50.0f, 0.0f, 70.0f);
	UPROPERTY(EditAnywhere, Category = "VisionSteal|Camera")
	FRotator CameraRelativeRotation = FRotator::ZeroRotator;

	FTimerHandle VisionStealTimerHandle;
	
	UFUNCTION()
	void HandleVisionStealTimeout();

	bool BeginVisionSteal(AActor* Target, float Duration, bool bConsumeCharge);
	
	UPROPERTY(EditAnywhere, Category = "VisionSteal")
	int32 MaxCharges = 3;

	UPROPERTY(EditAnywhere, Category = "VisionSteal")
	float VisionStealDuration = 4.0f;

	UPROPERTY(VisibleAnywhere, Category = "VisionSteal")
	int32 CurrentCharges = 3;

	UPROPERTY(VisibleAnywhere, Category = "VisionSteal")
	bool bForcedVisionSteal = false;
};
