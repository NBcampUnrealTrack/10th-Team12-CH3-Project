#include "AIBossCharacter.h"

#include "ForgottenGameMode.h"
#include "HealthComponent.h"
#include "Animation/AnimMontage.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "ForgottenGameState.h"
#include "VisionStealComponent.h"

namespace
{
constexpr float DefaultSecondPhaseHealthRatio = 0.5f;
constexpr float DefaultSecondPhaseSpeedMultiplier = 1.8f;
constexpr float DefaultMeleeRange = 300.0f;
constexpr float DefaultDashRange = 1500.0f;
constexpr float BossZeroThreshold = 0.0f;
constexpr int32 InvalidPatternIndex = -1;
constexpr int32 PatternSingleStep = 1;
constexpr float DefaultStaggerThreshold = 100.0f;
constexpr float DefaultStaggerDuration = 1.5f;
constexpr float DefaultStaggerImmunityDuration = 6.0f;
constexpr float StaggerRatioMax = 1.0f;
constexpr int32 BossPhaseOneStencilValue = 1;
constexpr int32 BossPhaseTwoStencilValue = 3;
constexpr float DefaultBossVisionStealDuration = 5.0f;
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
	StunDuration = 1.0f;
	StaggerThreshold = DefaultStaggerThreshold;
	StaggerDuration = DefaultStaggerDuration;
	StaggerImmunityDuration = DefaultStaggerImmunityDuration;
	StaggerGauge = BossZeroThreshold;
	LastBossHealth = BossZeroThreshold;
	bStaggerImmune = false;
	bShowHealthBar = true;
	VisionStealDuration = DefaultBossVisionStealDuration;
}

void AAIBossCharacter::BeginPlay()
{
	Super::BeginPlay();

	GetMesh()->SetCustomDepthStencilValue(BossPhaseOneStencilValue);
	GetMesh()->SetRenderCustomDepth(true);

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

	if (!bShowHealthBar)
	{
		return;
	}

	AForgottenGameState* ForgottenGameState = GetWorld()->GetGameState<AForgottenGameState>();

	if (!ForgottenGameState)
	{
		return;
	}

	ForgottenGameState->NotifyBossHealth(BossHealth->GetCurrentHealth(), BossHealth->GetMaxHealth());
	ForgottenGameState->NotifyBossActive(true);
}


void AAIBossCharacter::HandleBossHealthChanged(float CurrentHealth, float MaxHealth)
{
	OnBossHealthChanged.Broadcast(CurrentHealth, MaxHealth);

	if (bShowHealthBar)
	{
		AForgottenGameState* ForgottenGameState = GetWorld()->GetGameState<AForgottenGameState>();

		if (ForgottenGameState)
		{
			ForgottenGameState->NotifyBossHealth(CurrentHealth, MaxHealth);
		}
	}

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

	if (CurrentHealth / MaxHealth > SecondPhaseHealthRatio)
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

	GetMesh()->SetCustomDepthStencilValue(BossPhaseTwoStencilValue);

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	if (!PlayerPawn)
	{
		return;
	}

	UVisionStealComponent* PlayerVisionSteal = PlayerPawn->FindComponentByClass<UVisionStealComponent>();

	if (!PlayerVisionSteal)
	{
		return;
	}

	PlayerVisionSteal->ForceVisionSteal(this, VisionStealDuration);
}

void AAIBossCharacter::HandleBossDeath(AActor* DeadOwner)
{
	if (bShowHealthBar)
	{
		AForgottenGameState* DeathGameState = GetWorld()->GetGameState<AForgottenGameState>();

		if (DeathGameState)
		{
			DeathGameState->NotifyBossActive(false);
		}
	}

	EnterBossPhase(EBossPhase::Defeated);

	GetMesh()->SetRenderCustomDepth(false);

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
