#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ScoreOnDeathComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class FORGOTTENVIGILANCE_API UScoreOnDeathComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UScoreOnDeathComponent();

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void HandleDeath(AActor* DeadOwner);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Score", meta = (AllowPrivateAccess = true))
	int32 ScoreValue;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Score", meta = (AllowPrivateAccess = true))
	bool bScoreRegistered;
};