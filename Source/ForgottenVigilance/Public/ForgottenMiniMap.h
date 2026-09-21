#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ForgottenMiniMap.generated.h"

class UCanvasPanel;
class UImage;
class AForgottenMiniMapCaptureActor;
class AOptimusPrimeCharacter;
class UForgottenTargetMarker;

UCLASS()
class FORGOTTENVIGILANCE_API UForgottenMiniMap : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> MarkerCanvas;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> PlayerIcon;
	UPROPERTY(EditDefaultsOnly, Category = "Minimap")
	TSubclassOf<UForgottenTargetMarker> MarkerClass;

	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	void InitMiniMap(AForgottenMiniMapCaptureActor* InCapture, AOptimusPrimeCharacter* InPlayer);

private:
	UFUNCTION()
	void HandleTargetDetected(AActor* Target);
	UFUNCTION()
	void HandleTargetLost(AActor* Target);

	void UnbindPlayerEvents();

	TMap<TWeakObjectPtr<AActor>, TObjectPtr<UForgottenTargetMarker>> Markers;

	TWeakObjectPtr<AForgottenMiniMapCaptureActor> Capture;
	TWeakObjectPtr<AOptimusPrimeCharacter> Player;
};
