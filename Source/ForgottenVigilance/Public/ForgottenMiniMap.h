#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ForgottenMiniMap.generated.h"

class UCanvasPanel;
class UImage;
class AMinimapCaptureActor;
class AOptimusPlayerCharacter;
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

	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	void InitMiniMap(AForgottenMiniMapCaptureActor* AOptimusPrimeCharacter* InPlayer);

};
