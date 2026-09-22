#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MainWeaponComponent.generated.h"

class USoundBase;
class USoundAttenuation;
class UFXSystemAsset;
class UFXSystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHeatChanged, float, CurrentHeat, float, MaxHeat);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOverheatStateChanged, bool, bIsOverheated);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShotFired, int32, MuzzleIndex);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShotHit, bool, bHitCharacter);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FORGOTTENVIGILANCE_API UMainWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMainWeaponComponent();

	void AttachToCharacterMesh(USkeletalMeshComponent* ParentMesh);
	void StartFire();
	void StopFire();
	bool IsOverheated() const;
	float GetHeatRatio() const;
	bool GetAimTarget(FVector& OutTarget) const;

	UPROPERTY(BlueprintAssignable, Category = "Weapon")
	FOnHeatChanged OnHeatChanged;

	UPROPERTY(BlueprintAssignable, Category = "Weapon")
	FOnOverheatStateChanged OnOverheatStateChanged;

	UPROPERTY(BlueprintAssignable, Transient, Category = "Weapon")
	FOnShotFired OnShotFired;
	
		UPROPERTY(BlueprintAssignable, Category = "Weapon")
    	FOnShotHit OnShotHit;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

private:
	void Fire();
	void AddHeat();
	void UpdateCooling(float DeltaTime);
	void SetOverheated(bool bNewOverheated);
	bool TraceForHit(FHitResult& OutHit, FVector& OutShotEnd) const;
	FVector GetMuzzleLocation() const;
	FName GetCurrentMuzzleSocket() const;
	void SelectMuzzleForShot();
	USkeletalMeshComponent* GetMuzzleMesh(FName SocketName) const;
	void PlayShotEffects(bool bHit, const FHitResult& HitResult, const FVector& ShotEnd);
	void PlayMuzzleFlash();
	void StopMuzzleEffect();
	void PlayTracer(const FVector& TracerStart, const FVector& TracerEnd) const;
	void PlayImpact(const FHitResult& HitResult) const;
	UFXSystemComponent* SpawnEffectAttached(UFXSystemAsset* Effect, USceneComponent* Parent, FName SocketName) const;
	UFXSystemComponent* SpawnEffectAtLocation(UFXSystemAsset* Effect, const FVector& Location, const FRotator& Rotation) const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Visual", meta = (AllowPrivateAccess = true))
	TObjectPtr<USkeletalMesh> WeaponMesh;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Visual", meta = (AllowPrivateAccess = true))
	FName AttachSocketName;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Visual", meta = (AllowPrivateAccess = true))
	TObjectPtr<USkeletalMeshComponent> WeaponMeshComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = true))
	float Damage;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = true))
	float FireInterval;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = true))
	float TraceDistance;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = true))
	float MaxHeat;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = true))
	float HeatPerShot;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = true))
	float CoolingRate;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = true))
	float CurrentHeat;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = true))
	bool bIsOverheated;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Sound", meta = (AllowPrivateAccess = true))
	TObjectPtr<USoundBase> FireSound;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Sound", meta = (AllowPrivateAccess = true))
	TObjectPtr<USoundBase> OverheatSound;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Sound", meta = (AllowPrivateAccess = true))
	TObjectPtr<USoundAttenuation> SoundAttenuation;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Sound", meta = (AllowPrivateAccess = true))
	float NoiseLoudness;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Sound", meta = (AllowPrivateAccess = true))
	float NoiseRange;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Muzzle", meta = (AllowPrivateAccess = true))
	TArray<FName> MuzzleSocketNames;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Muzzle", meta = (AllowPrivateAccess = true))
	bool bRandomizeMuzzle;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|VFX", meta = (AllowPrivateAccess = true))
	TObjectPtr<UFXSystemAsset> MuzzleEffect;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|VFX", meta = (AllowPrivateAccess = true))
	TObjectPtr<UFXSystemAsset> ImpactWorldEffect;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|VFX", meta = (AllowPrivateAccess = true))
	TObjectPtr<UFXSystemAsset> ImpactCharacterEffect;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|VFX", meta = (AllowPrivateAccess = true))
	TObjectPtr<UFXSystemAsset> TracerEffect;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|VFX", meta = (AllowPrivateAccess = true))
	FName TracerEndParameterName;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|VFX", meta = (AllowPrivateAccess = true))
	FVector MuzzleEffectScale;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Muzzle", meta = (AllowPrivateAccess = true))
	int32 CurrentMuzzleIndex;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|VFX", meta = (AllowPrivateAccess = true))
	float MuzzleEffectLifetime;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|VFX", meta = (AllowPrivateAccess = true))
	TObjectPtr<UFXSystemComponent> ActiveMuzzleComponent;

	FTimerHandle FireTimerHandle;
	FTimerHandle MuzzleStopTimerHandle;
};
