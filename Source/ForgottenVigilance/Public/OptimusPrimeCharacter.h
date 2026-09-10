#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "OptimusPrimeCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UHealthComponent;
class UMainWeaponComponent;

struct FInputActionValue;


UCLASS()
class FORGOTTENVIGILANCE_API AOptimusPrimeCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintPure, Category = "Health")
	bool IsCharacterDead() const;

	AOptimusPrimeCharacter();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void Move(const FInputActionValue& Value);
	UFUNCTION()
	void StartJump(const FInputActionValue& Value);
	UFUNCTION()
	void StopJump(const FInputActionValue& Value);
	UFUNCTION()
	void Look(const FInputActionValue& Value);
	UFUNCTION()
	void StartSprint(const FInputActionValue& Value);
	UFUNCTION()
	void StopSprint(const FInputActionValue& Value);
	UFUNCTION()
	void FireWeapon(const FInputActionValue& Value);
	UFUNCTION()
	void StopFireWeapon(const FInputActionValue& Value);
	UFUNCTION()
	void StartCameraRotate(const FInputActionValue& Value);
	UFUNCTION()
	void StopCameraRotate(const FInputActionValue& Value);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArmComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> CameraComp;

private:
	UFUNCTION()
	void HandleDeath(AActor* DeadOwner);

	void UpdateSpeed();

	UPROPERTY(VisibleAnywhere, Category = "Health")
	TObjectPtr<UHealthComponent> HealthComp;
	UPROPERTY(VisibleAnywhere, Category = "Weapon")
	TObjectPtr<UMainWeaponComponent> MainWeaponComponent;
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float NormalSpeed;
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float SprintMultiplier;

	bool bIsSprinting;
	bool bIsCameraRotating;
};
