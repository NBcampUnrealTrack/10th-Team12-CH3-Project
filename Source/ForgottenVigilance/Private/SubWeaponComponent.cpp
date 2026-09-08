#include "SubWeaponComponent.h"

USubWeaponComponent::USubWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}

void USubWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

}

void USubWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}
