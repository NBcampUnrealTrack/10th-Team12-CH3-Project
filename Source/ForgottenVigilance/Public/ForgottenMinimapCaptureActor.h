#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ForgottenMinimapCaptureActor.generated.h"

class USceneCaptureComponent2D;
class UTextureRenderTarget2D;

UCLASS()
class FORGOTTENVIGILANCE_API AForgottenMinimapCaptureActor : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "MiniMap")
	TObjectPtr<UTextureRenderTarget2D> MiniMapRenderTarget;

	UFUNCTION(BlueprintPure, Category = "MiniMap")
	FVector2D WorldToMiniMapUV(const FVector& WorldLocation) const;

	AForgottenMinimapCaptureActor();

	UPROPERTY(VisibleAnywhere, Category = "Minimap")
	TObjectPtr<USceneCaptureComponent2D> CaptureComponent;

	UPROPERTY(EditAnywhere, Category = "MiniMap")
	float CeilingClipHeight = 3850.f;

	UPROPERTY(EditAnywhere, Category = "MiniMap")
	float OrthoWidth = 2048.0f;

	UPROPERTY(EditAnywhere, Category = "MiniMap")
	float CaptureHeight = 2048.0f;

protected:
	virtual void BeginPlay() override;
};
