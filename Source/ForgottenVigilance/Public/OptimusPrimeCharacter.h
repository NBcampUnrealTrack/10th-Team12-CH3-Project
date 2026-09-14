#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "OptimusPrimeCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UHealthComponent;
class UMainWeaponComponent;

struct FInputActionValue;

UENUM(BlueprintType)
enum class EPlayerCameraMode : uint8
{
	CharacterOrbit UMETA(DisplayName = "Character Orbit"),
	HorizontalDeadZone UMETA(DisplayName = "Horizontal Dead Zone"),
	ScreenPositionDeadZone UMETA(DisplayName = "Dead Zone - Keep Screen Position")
};

UCLASS()
class FORGOTTENVIGILANCE_API AOptimusPrimeCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintPure, Category = "Health")
	bool IsCharacterDead() const;

	UFUNCTION(BlueprintPure, Category = "Camera|DeadZone")
	float GetCameraDeadZoneHalfWidth() const;

	AOptimusPrimeCharacter();
	bool GetCameraDeadZoneBounds(FVector& LeftBoundary, FVector& RightBoundary) const;

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArmComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> CameraComp;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	float CharacterTargetArmLength;

private:
	UFUNCTION()
	void HandleDeath(AActor* DeadOwner);

	void UpdateSpeed();
	void InitializeCameraDeadZone(float DeltaTime);
	void UpdateCameraFollow();
	void UpdateCrosshairRotation(float DeltaTime);

	UPROPERTY(EditDefaultsOnly, Category = "Camera|Aiming")
	bool bRotateTowardCrosshair = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera",
		meta = (AllowPrivateAccess = "true"))
	EPlayerCameraMode CameraMode = EPlayerCameraMode::ScreenPositionDeadZone;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|DeadZone",
		meta = (AllowPrivateAccess = "true", ClampMin = "0.0", UIMax = "2.0"))
	float DeadZoneWidthScale = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Camera|DeadZone", meta = (ClampMin = "1.0", Units = "cm"))
	float CameraTeleportResetDistance = 2000.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Camera|DeadZone", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ReferenceDeadZoneScreenFraction = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Camera|DeadZone")
	FIntPoint DeadZoneReferenceResolution = FIntPoint(1920, 1032);

	UPROPERTY(VisibleInstanceOnly, Category = "Camera|DeadZone")
	float DeadZoneHalfWidthWorld = 0.0f;

	UPROPERTY(VisibleAnywhere, Category = "Health")
	TObjectPtr<UHealthComponent> HealthComp;
	UPROPERTY(VisibleAnywhere, Category = "Weapon")
	TObjectPtr<UMainWeaponComponent> MainWeaponComponent;
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float NormalSpeed;
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float SprintMultiplier;
	
	bool bIsSprinting;

	FVector CameraPivotWorldLocation;
	FVector PreviousCameraTargetLocation = FVector::ZeroVector;
	FVector InitialCameraPivotOffset = FVector::ZeroVector;
	float PreviousCameraYaw = 0.0f;
	bool bDeadZoneWidthInitialized = false;
};
