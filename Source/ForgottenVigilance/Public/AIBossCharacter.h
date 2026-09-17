#pragma once

#include "CoreMinimal.h"
#include "AIBaseCharacter.h"
#include "AIBossCharacter.generated.h"

class UAnimMontage;
class UHealthComponent;

UENUM(BlueprintType)
enum class EBossPhase : uint8
{
	First,
	Second,
	Defeated
};

USTRUCT(BlueprintType)
struct FBossAttackPattern
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss")
	TObjectPtr<UAnimMontage> Montage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss")
	float Damage = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss")
	float HitRadius = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss")
	float HitForwardOffset = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss")
	float RequiredRange = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss")
	bool bSecondPhaseOnly = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBossPhaseChanged, EBossPhase, NewPhase);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBossHealthChanged, float, CurrentHealth, float, MaxHealth);

UCLASS()
class FORGOTTENVIGILANCE_API AAIBossCharacter : public AAIBaseCharacter
{
	GENERATED_BODY()

public:
	AAIBossCharacter();

	float ExecuteAttackPattern(AActor* TargetActor);
	void ApplyCurrentPatternDamage();
	bool IsFinalBoss() const;
	EBossPhase GetBossPhase() const;
	float GetMeleeRange() const;
	float GetDashRange() const;

	UPROPERTY(BlueprintAssignable, Category = "Boss")
	FOnBossPhaseChanged OnBossPhaseChanged;

	UPROPERTY(BlueprintAssignable, Category = "Boss")
	FOnBossHealthChanged OnBossHealthChanged;

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void HandleBossHealthChanged(float CurrentHealth, float MaxHealth);

	UFUNCTION()
	void HandleBossDeath(AActor* DeadOwner);

	void EnterBossPhase(EBossPhase NewPhase);
	int32 SelectPatternIndex(float DistanceToTarget) const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss", meta = (AllowPrivateAccess = true))
	TArray<FBossAttackPattern> AttackPatterns;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss", meta = (AllowPrivateAccess = true))
	bool bIsFinalBoss;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss", meta = (AllowPrivateAccess = true))
	float SecondPhaseHealthRatio;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss", meta = (AllowPrivateAccess = true))
	float SecondPhaseSpeedMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss", meta = (AllowPrivateAccess = true))
	float MeleeRange;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss", meta = (AllowPrivateAccess = true))
	float DashRange;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss", meta = (AllowPrivateAccess = true))
	EBossPhase BossPhase;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss", meta = (AllowPrivateAccess = true))
	int32 CurrentPatternIndex;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss", meta = (AllowPrivateAccess = true))
	int32 AttackCounter;
};