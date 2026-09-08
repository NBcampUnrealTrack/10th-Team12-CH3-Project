
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "OptimusPrimeCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
struct FInputActionValue;

UCLASS()
class FORGOTTENVIGILANCE_API AOptimusPrimeCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	AOptimusPrimeCharacter();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void Move(const FInputActionValue& value);
	UFUNCTION()
	void StartJump(const FInputActionValue& value);
	UFUNCTION()
	void StopJump(const FInputActionValue& value);
	UFUNCTION()
	void Look(const FInputActionValue& value);
	UFUNCTION()
	void StartSprint(const FInputActionValue& value);
	UFUNCTION()
	void StopSprint(const FInputActionValue& value);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArmComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> CameraComp;

private:
	void UpdateSpeed();

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float NormalSpeed;
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float SprintMultiplier;

	bool bIsSprinting;
};
