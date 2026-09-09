#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MainWeaponComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHeatChanged, float, CurrentHeat, float, MaxHeat);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOverheatStateChanged, bool, bIsOverheated);

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

	UPROPERTY(BlueprintAssignable, Category = "Weapon")
	FOnHeatChanged OnHeatChanged;

	UPROPERTY(BlueprintAssignable, Category = "Weapon")
	FOnOverheatStateChanged OnOverheatStateChanged;

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
	bool TraceForHit(FHitResult& OutHit) const;
	FVector GetMuzzleLocation() const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Visual", meta = (AllowPrivateAccess = true))
	TObjectPtr<USkeletalMesh> WeaponMesh;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Visual", meta = (AllowPrivateAccess = true))
	FName AttachSocketName;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Visual", meta = (AllowPrivateAccess = true))
	FName MuzzleSocketName;
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

	FTimerHandle FireTimerHandle;
};
