#include "HealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

namespace
{
constexpr float DefaultMaxHealth = 100.0f;
constexpr float HealthZeroThreshold = 0.0f;
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
	if (Damage <= HealthZeroThreshold)
	{
		return;
	}

	if (!IsAlive())
	{
		return;
	}

	CurrentHealth = FMath::Clamp(CurrentHealth - Damage, HealthZeroThreshold, MaxHealth);
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);

	if (IsAlive())
	{
		PlayHealthSound(HitSound);
		return;
	}

	PlayHealthSound(DeathSound);
	OnDeath.Broadcast(GetOwner());
}

void UHealthComponent::PlayHealthSound(USoundBase* SoundToPlay) const
{
	if (!SoundToPlay)
	{
		return;
	}

	const AActor* OwnerActor = GetOwner();

	if (!OwnerActor)
	{
		return;
	}

	UGameplayStatics::PlaySoundAtLocation(
		this,
		SoundToPlay,
		OwnerActor->GetActorLocation(),
		1.0f,
		1.0f,
		0.0f,
		SoundAttenuation);
}

void UHealthComponent::Heal(float HealAmount)
{
	if (HealAmount <= HealthZeroThreshold)
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
	return CurrentHealth > HealthZeroThreshold;
}

float UHealthComponent::GetCurrentHealth() const
{
	return CurrentHealth;
}

float UHealthComponent::GetMaxHealth() const
{
	return MaxHealth;
}

bool UHealthComponent::IsHit() const
{
	return CurrentHealth < MaxHealth;
}
