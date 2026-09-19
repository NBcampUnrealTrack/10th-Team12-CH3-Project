
#include "VisionStealComponent.h"

void UVisionStealComponent::ToggleVisionSteal()
{
	UE_LOG(LogTemp, Warning, TEXT("ToggleVisionSteal"));
}

UVisionStealComponent::UVisionStealComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}


void UVisionStealComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UVisionStealComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

