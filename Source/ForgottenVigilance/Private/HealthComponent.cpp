#include "HealthComponent.h"

namespace
{
constexpr float DefaultMaxHealth = 100.0f;
constexpr float ZeroThreshold = 0.0f;
}

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	MaxHealth = DefaultMaxHealth;
	CurrentHealth = DefaultMaxHealth;
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	OwnerActor->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::HandleTakeAnyDamage);
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

void UHealthComponent::HandleTakeAnyDamage(
	AActor* DamagedActor,
	float Damage,
	const UDamageType* DamageType,
	AController* InstigatedBy,
	AActor* DamageCauser)
{
	if (Damage <= ZeroThreshold)
	{
		return;
	}

	if (IsAlive() == false)
	{
		return;
	}

	CurrentHealth = FMath::Clamp(CurrentHealth - Damage, ZeroThreshold, MaxHealth);
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);

	if (IsAlive())
	{
		OnDeath.Broadcast(GetOwner());
	}
}

void UHealthComponent::Heal(float HealAmount)
{
	if (HealAmount <= ZeroThreshold)
	{
		return;
	}

	if (!IsAlive())
	{
		return;
	}

	CurrentHealth = FMath::Min(CurrentHealth + HealAmount, MaxHealth);
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

bool UHealthComponent::IsAlive() const
{
	return CurrentHealth > ZeroThreshold;
}

float UHealthComponent::GetCurrentHealth() const
{
	return CurrentHealth;
}

float UHealthComponent::GetMaxHealth() const
{
	return MaxHealth;
}
