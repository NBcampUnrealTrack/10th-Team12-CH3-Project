#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AIRangeWeaponComponent.generated.h"

class UFXSystemAsset;
class UFXSystemComponent;
class USkeletalMeshComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FORGOTTENVIGILANCE_API UAIRangeWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAIRangeWeaponComponent();

	void FireGun();

private:
	
	UPROPERTY(EditAnywhere, Category = "Weapon")
	float AIGunDamage = 20.0f;

	UPROPERTY(EditAnywhere, Category = "Weapon")
	float AIGunAttackRange = 1000.0f;


	//effect

	bool AITraceForHit(FHitResult& OutHit, FVector& OutShotEnd) const;

	FVector GetMuzzleLocation() const;
	
	USkeletalMeshComponent* GetMuzzleMesh() const;

	void PlayShotEffects(const FVector& ShotEnd);
	
	void PlayMuzzleFlash();
	
	void StopMuzzleEffect();
	
	void PlayTracer(const FVector& TracerStart, const FVector& TracerEnd) const;

	
	UFXSystemComponent* SpawnEffectAttached(UFXSystemAsset* Effect, USceneComponent* Parent, FName SocketName) const;
	
	UFXSystemComponent* SpawnEffectAtLocation(UFXSystemAsset* Effect, const FVector& Location, const FRotator& Rotation) const;



	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Muzzle", meta = (AllowPrivateAccess = "true"))
	FName MuzzleSocketName = TEXT("muzzle");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|VFX", meta = (AllowPrivateAccess = true))
	TObjectPtr<UFXSystemAsset> MuzzleEffect;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|VFX", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UFXSystemAsset> TracerEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|VFX", meta = (AllowPrivateAccess = "true"))
	FName TracerEndParameterName = TEXT("BeamEnd");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|VFX", meta = (AllowPrivateAccess = "true"))
	FVector MuzzleEffectScale = FVector::OneVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|VFX", meta = (AllowPrivateAccess = "true"))
	float MuzzleEffectLifetime = 0.08f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|VFX", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UFXSystemComponent> ActiveMuzzleComponent;

	FTimerHandle MuzzleStopTimerHandle;

};
