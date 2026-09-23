
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VisionStealComponent.generated.h"

class AAIBaseCharacter;
class ACameraActor;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class FORGOTTENVIGILANCE_API UVisionStealComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION()
	void ToggleVisionSteal();
	UFUNCTION(BlueprintCallable)
	bool StartVisionSteal(AActor* Target);
	UFUNCTION(BlueprintCallable)
	void EndVisionSteal();

	bool IsVisionStealActive() const { return bVisionStealActive; }

	UVisionStealComponent();

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
};
