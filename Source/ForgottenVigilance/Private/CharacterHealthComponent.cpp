#include "CharacterHealthComponent.h"

namespace
{
constexpr float DefaultMaxHealth = 100.0f;
constexpr float ZeroThreshold = 0.0f;
}

UCharacterHealthComponent::UCharacterHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	MaxHealth = DefaultMaxHealth;
	CurrentHealth = DefaultMaxHealth;
}

void UCharacterHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	OwnerActor->OnTakeAnyDamage.AddDynamic(this, &UCharacterHealthComponent::HandleTakeAnyDamage);
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

void UCharacterHealthComponent::HandleTakeAnyDamage(
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

void UCharacterHealthComponent::Heal(float HealAmount)
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

bool UCharacterHealthComponent::IsAlive() const
{
	return CurrentHealth > ZeroThreshold;
}

float UCharacterHealthComponent::GetCurrentHealth() const
{
	return CurrentHealth;
}

float UCharacterHealthComponent::GetMaxHealth() const
{
	return MaxHealth;
}
