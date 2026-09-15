#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AIBaseCharacter.generated.h"

class UHealthComponent;

UCLASS()
class FORGOTTENVIGILANCE_API AAIBaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AAIBaseCharacter();
	void SetMovementSpeed(float NewSpeed);

	UPROPERTY(EditAnywhere, Category = "AI")
	float WalkSpeed = 300.0f;

	UPROPERTY(EditAnywhere, Category = "AI")
	float RunSpeed = 600.0f;
	
protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void HandleDeath(AActor* DeadOwner);

	UPROPERTY(EditDefaultsOnly, Category = "Health")
	TObjectPtr<UHealthComponent> HealthComponent;
};
