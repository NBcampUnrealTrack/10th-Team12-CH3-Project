#include "ForgottenMiniMap.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "ForgottenMiniMapCaptureActor.h"
#include "ForgottenTargetMarker.h"
#include "OptimusPrimeCharacter.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Styling/SlateBrush.h"

void UForgottenMiniMap::InitMiniMap(AForgottenMiniMapCaptureActor* InCapture, AOptimusPrimeCharacter* InPlayer)
{
	if (!InCapture || !InPlayer)
	{
		UE_LOG(LogTemp, Warning, TEXT("InitMiniMap Invalid Parameter"));
		return;
	}

	UnbindPlayerEvents();
	for (const TPair<TWeakObjectPtr<AActor>, TObjectPtr<UForgottenTargetMarker>>& Pair : Markers)
	{
		if (Pair.Value)
		{
			Pair.Value->RemoveFromParent();
		}
	}
	Markers.Empty();

	Capture = InCapture;
	Player = InPlayer;

	InPlayer->OnTargetDetected.AddDynamic(this, &UForgottenMiniMap::HandleTargetDetected);
	InPlayer->OnTargetLost.AddDynamic(this, &UForgottenMiniMap::HandleTargetLost);

	TArray<AActor*> ExistingTargets;
	InPlayer->GetDetectedTargets(ExistingTargets);
	for (AActor* Target : ExistingTargets)
	{
		HandleTargetDetected(Target);
	}
}

void UForgottenMiniMap::NativeConstruct()
{
	Super::NativeConstruct();

	if (MiniMap && MiniMapRenderTarget)
	{
		FSlateBrush MiniMapBrush;
		MiniMapBrush.SetResourceObject(MiniMapRenderTarget);

		MiniMap->SetBrush(MiniMapBrush);
	}
}

void UForgottenMiniMap::NativeDestruct()
{
	UnbindPlayerEvents();
	Super::NativeDestruct();
}

void UForgottenMiniMap::UnbindPlayerEvents()
{
	if (Player.IsValid())
	{
		Player->OnTargetDetected.RemoveAll(this);
		Player->OnTargetLost.RemoveAll(this);
	}
}

void UForgottenMiniMap::HandleTargetDetected(AActor* Target)
{
	UE_LOG(LogTemp, Warning, TEXT("HandleTargetDetected: Target=%s MarkerClass=%s MarkerCanvas=%s"),
	    *GetNameSafe(Target), *GetNameSafe(MarkerClass), *GetNameSafe(MarkerCanvas));

	if (!Target || !MarkerClass || !MarkerCanvas || Markers.Contains(Target))
	{
		return;
	}

	UForgottenTargetMarker* Marker = CreateWidget<UForgottenTargetMarker>(this, MarkerClass);
	if (!Marker)
	{
		return;
	}
	Marker->SetTarget(Target);

	if (UCanvasPanelSlot* CanvasSlot = MarkerCanvas->AddChildToCanvas(Marker))
	{
		CanvasSlot->SetAnchors(FAnchors(0.f, 0.f));
		CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		CanvasSlot->SetAutoSize(true);
	}

	Marker->SetVisibility(ESlateVisibility::Collapsed);
	Markers.Add(Target, Marker);
}

void UForgottenMiniMap::HandleTargetLost(AActor* Target)
{
	TObjectPtr<UForgottenTargetMarker> Marker;
	if (Markers.RemoveAndCopyValue(Target, Marker) && Marker)
	{
		Marker->RemoveFromParent();
	}
}

void UForgottenMiniMap::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!Capture.IsValid() || !MarkerCanvas)
	{
		return;
	}

	const FVector2D MapSize = MarkerCanvas->GetCachedGeometry().GetLocalSize();
	if (MapSize.X <= 0.f)
	{
		return;
	}

	if (PlayerIcon && Player.IsValid())
	{
		PlayerIcon->SetRenderTransformAngle(Player->GetActorRotation().Yaw - Capture->GetActorRotation().Yaw);
	}

	TArray<TWeakObjectPtr<AActor>> StaleKeys;

	for (const TPair<TWeakObjectPtr<AActor>, TObjectPtr<UForgottenTargetMarker>>& Pair : Markers)
	{
		AActor* Target = Pair.Key.Get();
		UForgottenTargetMarker* Marker = Pair.Value;

		if (!Target || !Marker)
		{
			StaleKeys.Add(Pair.Key);
			continue;
		}

		FVector2D MapPosition;
		const bool bInRange = Capture->WorldToMiniMap(Target->GetActorLocation(), MapSize, MapPosition);

		Marker->SetVisibility(bInRange ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);

		if (bInRange)
		{
			if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Marker->Slot))
			{
				CanvasSlot->SetPosition(MapPosition);
			}
		}
	}

	for (const TWeakObjectPtr<AActor>& StaleKey : StaleKeys)
	{
		TObjectPtr<UForgottenTargetMarker> Marker;
		if (Markers.RemoveAndCopyValue(StaleKey, Marker) && Marker)
		{
			Marker->RemoveFromParent();
		}
	}
}