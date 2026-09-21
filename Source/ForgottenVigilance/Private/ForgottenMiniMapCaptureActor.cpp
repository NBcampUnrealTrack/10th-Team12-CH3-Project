#include "ForgottenMiniMapCaptureActor.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"


AForgottenMiniMapCaptureActor::AForgottenMiniMapCaptureActor()
{
	PrimaryActorTick.bCanEverTick = false;

	CaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("CaptureComponent"));
	RootComponent = CaptureComponent;

	CaptureComponent->ProjectionType = ECameraProjectionMode::Orthographic;
	CaptureComponent->OrthoWidth = OrthoWidth;
	CaptureComponent->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;

	CaptureComponent->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));

	CaptureComponent->bEnableClipPlane = true;
	CaptureComponent->ClipPlaneNormal = FVector(0.f, 0.f, -1.f);
	CaptureComponent->ClipPlaneBase = FVector(0.f, 0.f, CeilingClipHeight);
}


void AForgottenMiniMapCaptureActor::BeginPlay()
{
	Super::BeginPlay();
	
	 if (MiniMapRenderTarget)
	{
		CaptureComponent->TextureTarget = MiniMapRenderTarget;
	}

	FVector Location = GetActorLocation();
	Location.Z = CaptureHeight;
	SetActorLocation(Location);
}


FVector2D AForgottenMiniMapCaptureActor::WorldToMiniMapUV(const FVector& WorldLocation) const
{
	const FVector CaptureOrigin = GetActorLocation();
	const float HalfWidth = OrthoWidth * 0.5f;

	const float U = (WorldLocation.X - CaptureOrigin.X + HalfWidth) / OrthoWidth;
	const float V = (WorldLocation.Y - CaptureOrigin.Y + HalfWidth) / OrthoWidth;

	return FVector2D(FMath::Clamp(U, 0.f, 1.f), FMath::Clamp(V, 0.f, 1.f));
}

bool AForgottenMiniMapCaptureActor::WorldToMiniMap(const FVector& WorldLocation, const FVector2D& MapSize, FVector2D& OutMapPosition) const
{
	const FVector Delta = WorldLocation - GetActorLocation();
	const FVector Local = FRotator(0.f, GetActorRotation().Yaw, 0.f).UnrotateVector(Delta);

	const float PixelsPerUnit = MapSize.X / CaptureComponent->OrthoWidth;
	const FVector2D Center = MapSize * 0.5f;
	OutMapPosition = Center + FVector2D(Local.Y, -Local.X) * PixelsPerUnit;

	return FVector2D::Distance(OutMapPosition, Center) <= MapSize.X * 0.5f;
}
