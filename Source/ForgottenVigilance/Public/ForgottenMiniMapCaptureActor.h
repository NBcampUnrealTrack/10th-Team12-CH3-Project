#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ForgottenMiniMapCaptureActor.generated.h"

class USceneCaptureComponent2D;
class UTextureRenderTarget2D;

UCLASS()
class FORGOTTENVIGILANCE_API AForgottenMiniMapCaptureActor : public AActor
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "MiniMap")
	bool WorldToMiniMap(const FVector& WorldLocation, const FVector2D& MapSize, FVector2D& OutMapPosition) const;

	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditAnywhere, Category = "MiniMap")
	TObjectPtr<UTextureRenderTarget2D> MiniMapRenderTarget;

	UFUNCTION(BlueprintPure, Category = "MiniMap")
	FVector2D WorldToMiniMapUV(const FVector& WorldLocation) const;

	AForgottenMiniMapCaptureActor();

	UPROPERTY(VisibleAnywhere, Category = "Minimap")
	TObjectPtr<USceneCaptureComponent2D> CaptureComponent;

	UPROPERTY(EditAnywhere, Category = "MiniMap")
	float CeilingClipHeight = 3850.f;

	UPROPERTY(EditAnywhere, Category = "MiniMap")
	float OrthoWidth = 2048.0f;

	UPROPERTY(EditAnywhere, Category = "MiniMap")
	float CaptureHeight = 2048.0f;
};
