#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AIRangeWeaponComponent.generated.h"

class UFXSystemAsset;
class UFXSystemComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FORGOTTENVIGILANCE_API UAIRangeWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAIRangeWeaponComponent();

	void FireGun();

private:
	bool AITraceForHit(FHitResult& OutHit) const;

	UPROPERTY(EditAnywhere, Category = "Weapon")
	float AIGunDamage = 20.0f;

	UPROPERTY(EditAnywhere, Category = "Weapon")
	float AIGunAttackRange = 1000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|VFX", meta = (AllowPrivateAccess = true))
	TObjectPtr<UFXSystemAsset> MuzzleEffect;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|VFX", meta = (AllowPrivateAccess = true))
	TObjectPtr<UFXSystemAsset> ImpactWorldEffect;

	UFXSystemComponent* SpawnEffectAttached(UFXSystemAsset* Effect, USceneComponent* Parent, FName SocketName) const;
	UFXSystemComponent* SpawnEffectAtLocation(UFXSystemAsset* Effect, const FVector& Location, const FRotator& Rotation) const;
};
