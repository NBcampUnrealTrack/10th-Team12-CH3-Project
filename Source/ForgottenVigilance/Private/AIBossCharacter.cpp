#include "AIBossCharacter.h"

#include "ForgottenGameMode.h"
#include "HealthComponent.h"
#include "Animation/AnimMontage.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"

namespace
{
constexpr float DefaultSecondPhaseHealthRatio = 0.5f;
constexpr float DefaultSecondPhaseSpeedMultiplier = 1.4f;
constexpr float DefaultMeleeRange = 300.0f;
constexpr float DefaultDashRange = 900.0f;
constexpr float BossZeroThreshold = 0.0f;
constexpr int32 InvalidPatternIndex = -1;
constexpr int32 PatternSingleStep = 1;
constexpr float DefaultStaggerThreshold = 200.0f;
constexpr float DefaultStaggerDuration = 1.5f;
constexpr float DefaultStaggerImmunityDuration = 6.0f;
constexpr float StaggerRatioMax = 1.0f;
}

AAIBossCharacter::AAIBossCharacter()
{
	bLoseTargetOnHit = false;

	bIsFinalBoss = false;
	SecondPhaseHealthRatio = DefaultSecondPhaseHealthRatio;
	SecondPhaseSpeedMultiplier = DefaultSecondPhaseSpeedMultiplier;
	MeleeRange = DefaultMeleeRange;
	DashRange = DefaultDashRange;
	BossPhase = EBossPhase::First;
	CurrentPatternIndex = InvalidPatternIndex;
	AttackCounter = 0;
	GetCharacterMovement()->bUseRVOAvoidance = false;
<<<<<<< Updated upstream
	StunDuration = 0.35f;
=======
	StunDuration = 1.0f;
	StaggerThreshold = DefaultStaggerThreshold;
	StaggerDuration = DefaultStaggerDuration;
	StaggerImmunityDuration = DefaultStaggerImmunityDuration;
	StaggerGauge = BossZeroThreshold;
	LastBossHealth = BossZeroThreshold;
	bStaggerImmune = false;
>>>>>>> Stashed changes
}

void AAIBossCharacter::BeginPlay()
{
	Super::BeginPlay();

	UHealthComponent* BossHealth = FindComponentByClass<UHealthComponent>();

	if (!BossHealth)
	{
		return;
	}
	
	LastBossHealth = BossHealth->GetCurrentHealth();

	BossHealth->OnHealthChanged.AddDynamic(this, &AAIBossCharacter::HandleBossHealthChanged);
	BossHealth->OnDeath.AddDynamic(this, &AAIBossCharacter::HandleBossDeath);

	OnBossHealthChanged.Broadcast(BossHealth->GetCurrentHealth(), BossHealth->GetMaxHealth());
	OnBossPhaseChanged.Broadcast(BossPhase);
}

void AAIBossCharacter::HandleBossHealthChanged(float CurrentHealth, float MaxHealth)
{
	OnBossHealthChanged.Broadcast(CurrentHealth, MaxHealth);

	const float DamageTaken = LastBossHealth - CurrentHealth;
	LastBossHealth = CurrentHealth;

	if (CurrentHealth > BossZeroThreshold)
	{
		AccumulateStagger(DamageTaken);
	}

	if (MaxHealth <= BossZeroThreshold)
	{
		return;
	}

	EnterBossPhase(EBossPhase::Second);
}

void AAIBossCharacter::AccumulateStagger(float DamageAmount)
{
	if (DamageAmount <= BossZeroThreshold || bStaggerImmune || BossPhase == EBossPhase::Defeated)
	{
		return;
	}

	StaggerGauge += DamageAmount;

	if (StaggerGauge < StaggerThreshold)
	{
		return;
	}

	StaggerGauge = BossZeroThreshold;
	bStaggerImmune = true;

	ApplyStun(StaggerDuration);

	GetWorldTimerManager().SetTimer(
		StaggerImmunityHandle,
		this,
		&AAIBossCharacter::ClearStaggerImmunity,
		StaggerImmunityDuration,
		false);
}

void AAIBossCharacter::ClearStaggerImmunity()
{
	bStaggerImmune = false;
}

float AAIBossCharacter::GetStaggerRatio() const
{
	if (StaggerThreshold <= BossZeroThreshold)
	{
		return BossZeroThreshold;
	}

	return FMath::Clamp(StaggerGauge / StaggerThreshold, BossZeroThreshold, StaggerRatioMax);
}

void AAIBossCharacter::EnterBossPhase(EBossPhase NewPhase)
{
	if (BossPhase == NewPhase)
	{
		return;
	}

	BossPhase = NewPhase;
	OnBossPhaseChanged.Broadcast(BossPhase);

	if (NewPhase != EBossPhase::Second)
	{
		return;
	}

	SetMovementSpeed(RunSpeed * SecondPhaseSpeedMultiplier);
}

void AAIBossCharacter::HandleBossDeath(AActor* DeadOwner)
{
	EnterBossPhase(EBossPhase::Defeated);

	if (!bIsFinalBoss)
	{
		return;
	}

	AForgottenGameMode* ForgottenGameMode = GetWorld()->GetAuthGameMode<AForgottenGameMode>();

	if (!ForgottenGameMode)
	{
		return;
	}

	ForgottenGameMode->NotifyFinalBossDefeated();
}

int32 AAIBossCharacter::SelectPatternIndex(float DistanceToTarget) const
{
	TArray<int32> Candidates;

	for (int32 PatternIndex = 0; PatternIndex < AttackPatterns.Num(); ++PatternIndex)
	{
		const FBossAttackPattern& Pattern = AttackPatterns[PatternIndex];

		if (!Pattern.Montage)
		{
			continue;
		}

		if (Pattern.bSecondPhaseOnly && BossPhase != EBossPhase::Second)
		{
			continue;
		}

		if (DistanceToTarget > Pattern.RequiredRange)
		{
			continue;
		}

		Candidates.Add(PatternIndex);
	}

	if (Candidates.Num() <= 0)
	{
		return InvalidPatternIndex;
	}

	return Candidates[AttackCounter % Candidates.Num()];
}

float AAIBossCharacter::ExecuteAttackPattern(AActor* TargetActor)
{
	if (!TargetActor || BossPhase == EBossPhase::Defeated)
	{
		return BossZeroThreshold;
	}

	const float DistanceToTarget = FVector::Dist(GetActorLocation(), TargetActor->GetActorLocation());
	const int32 SelectedIndex = SelectPatternIndex(DistanceToTarget);

	if (SelectedIndex == InvalidPatternIndex)
	{
		return BossZeroThreshold;
	}

	CurrentPatternIndex = SelectedIndex;
	AttackCounter += PatternSingleStep;

	return PlayAnimMontage(AttackPatterns[SelectedIndex].Montage);
}

void AAIBossCharacter::ApplyCurrentPatternDamage()
{
	if (!AttackPatterns.IsValidIndex(CurrentPatternIndex))
	{
		return;
	}

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	if (!PlayerPawn)
	{
		return;
	}

	const FBossAttackPattern& Pattern = AttackPatterns[CurrentPatternIndex];
	const FVector HitLocation = GetActorLocation() + GetActorForwardVector() * Pattern.HitForwardOffset;

	if (FVector::Dist(HitLocation, PlayerPawn->GetActorLocation()) > Pattern.HitRadius)
	{
		return;
	}

	UGameplayStatics::ApplyDamage(PlayerPawn, Pattern.Damage, GetController(), this, nullptr);
}

bool AAIBossCharacter::IsFinalBoss() const
{
	return bIsFinalBoss;
}

EBossPhase AAIBossCharacter::GetBossPhase() const
{
	return BossPhase;
}

float AAIBossCharacter::GetMeleeRange() const
{
	return MeleeRange;
}

float AAIBossCharacter::GetDashRange() const
{
	return DashRange;
}