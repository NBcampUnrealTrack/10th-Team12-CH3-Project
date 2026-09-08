
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "OptimusPrimeCharacter.generated.h"

class USpringArmComponent; // 스프링 암 관련 클래스 헤더
class UCameraComponent;    // 카메라 관련 클래스 전방 선언
struct FInputActionValue;

UCLASS()
class FORGOTTENVIGILANCE_API AOptimusPrimeCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AOptimusPrimeCharacter();

	void UpdateSpeed();

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArmComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> CameraComp;

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

private:
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float NormalSpeed;
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float SprintMultiplier;

	bool bIsSprinting;
};
