#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, float, CurrentHealth, float, MaxHealth);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeath, AActor*, DeadOwner);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FORGOTTENVIGILANCE_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHealthComponent();

	void Heal(float HealAmount);
	bool IsAlive() const;
	float GetCurrentHealth() const;
	float GetMaxHealth() const;
	bool IsHit() const;

	UPROPERTY(BlueprintAssignable, Transient, Category = Health)
	FOnHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Transient, Category = Health)
	FOnDeath OnDeath;

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION() //OnTakeAnyDamage 구독용
	void HandleTakeAnyDamage(
		AActor* DamagedActor,
		float Damage,
		const UDamageType* DamageType,
		AController* InstigatedBy,
		AActor* DamageCauser);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Health, meta = (AllowPrivateAccess = true))
	float MaxHealth;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Health, meta = (AllowPrivateAccess = true))
	float CurrentHealth;
};
