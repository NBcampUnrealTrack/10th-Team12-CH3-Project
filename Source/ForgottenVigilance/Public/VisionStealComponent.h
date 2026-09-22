
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
	FName CameraAttachSocketName = TEXT("head");
	UPROPERTY(EditAnywhere, Category = "VisionSteal|Camera")
	FVector CameraRelativeLocation;
	UPROPERTY(EditAnywhere, Category = "VisionSteal|Camera")
	FRotator CameraRelativeRotation;

	FTimerHandle VisionStealTimerHandle;
};
