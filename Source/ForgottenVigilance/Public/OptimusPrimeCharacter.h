#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "OptimusPrimeCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeathAnimationReady);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetDetercted, AActor*, Target);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetLost, AActor*, Target);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerDamaged);

class USpringArmComponent;
class UCameraComponent;
class UHealthComponent;
class UVisionStealComponent;
class UMainWeaponComponent;
class UPawnSensingComponent;
class UAnimMontage;
class UAnimSequence;
class UCameraShakeBase;

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

	UFUNCTION(BlueprintPure, Category = "Camera|DeadZone")
	float GetCameraDeadZoneHalfWidth() const;

	UPROPERTY(BlueprintAssignable, Transient, Category = "Animation|Death")
	FOnDeathAnimationReady OnDeathAnimationReady;
	UFUNCTION(BlueprintCallable, Category = "Animation|Death")
	void EnableDeathRagdoll();

	UPROPERTY(BlueprintAssignable, Category = "Target")
	FOnTargetDetercted OnTargetDetected;
	UPROPERTY(BlueprintAssignable, Category = "Target")
	FOnTargetLost OnTargetLost;
	UFUNCTION(BlueprintPure, Category = "Target")
	AActor* GetCurrentTarget() const;
	UFUNCTION(BlueprintCallable, Category = "Target")
	void ClearCurrentTarget();
	UFUNCTION(BlueprintCallable, Category = "Target")
	void GetDetectedTargets(TArray<AActor*>& OutDetectedTargets) const;
	
	UPROPERTY(BlueprintAssignable, Category = "Damage Feedback")
	FOnPlayerDamaged OnPlayerDamaged;

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
	void Interact(const FInputActionValue& Value);
	UFUNCTION()
	void StartCrouch(const FInputActionValue& Value);
	UFUNCTION()
	void StopCrouch(const FInputActionValue& Value);
	UFUNCTION()
	void StartDash(const FInputActionValue& Value);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArmComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> CameraComp;

private:
	UFUNCTION()
	void HandleDeath(AActor* DeadOwner);
	UFUNCTION()
	void HandleShotFired(int32 MuzzleIndex);
	
	UFUNCTION()
	void HandleHealthChanged(float CurrentHealth, float MaxHealth);

	UFUNCTION()
	void EndHitStop();

	void UpdateDash(float DeltaTime);
	void EndDash();
	UFUNCTION()
	void OnPawnDetected(APawn* DetectedPawn);

	void UpdateSpeed();
	void UpdateCameraDeadZoneWidth(float DeltaTime);
	void UpdateCameraFollow();
	void UpdateCrosshairRotation(float DeltaTime);
	void CheckStaleTargets();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|DeadZone",
	    meta = (AllowPrivateAccess = "true", ClampMin = "0.0",
	        ToolTip = "Width relative to the reference screen: 0.1 = 10%, 1 = 100%. Values above 1 are allowed. Follows Target Arm Length and camera FOV changes, independently of viewport resizing and spring arm collision."))
	float DeadZoneWidthFraction = 0.1f;

	UPROPERTY(EditDefaultsOnly, Category = "Camera|DeadZone", meta = (ClampMin = "1.0", Units = "cm"))
	float CameraTeleportResetDistance = 2000.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Camera|DeadZone")
	FIntPoint DeadZoneReferenceResolution = FIntPoint(1920, 1032);

	UPROPERTY(VisibleInstanceOnly, Category = "Camera|DeadZone")
	float DeadZoneReferenceHalfWidthWorld = 0.0f;

	UPROPERTY(VisibleAnywhere, Category = "Health")
	TObjectPtr<UHealthComponent> HealthComp;
	UPROPERTY(VisibleAnywhere, Category = "Weapon")
	TObjectPtr<UMainWeaponComponent> MainWeaponComponent;
	UPROPERTY(VisibleAnywhere, Category = "Ability|VisionSteal")
	TObjectPtr<UVisionStealComponent> VisionStealComponent;
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float NormalSpeed;
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float NonForwardSpeedMultiplier;
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float SprintMultiplier;

	UPROPERTY(EditDefaultsOnly, Category = "Movement|Dash", meta = (ClampMin = "0.0"))
	float DashDistance = 200.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Movement|Dash",
	    meta = (ClampMin = "0.01", ToolTip = "Time to travel the distance above. Shorter means a faster dash."))
	float DashDuration = 0.25f;
	UPROPERTY(EditDefaultsOnly, Category = "Movement|Dash", meta = (ClampMin = "0.0"))
	float DashCooldown = 2.0f;

	bool bIsDashing = false;
	bool bDashOnCooldown = false;
	float DashElapsedTime = 0.0f;
	FVector DashDirection = FVector::ZeroVector;
	FVector2D CurrentMoveInput = FVector2D::ZeroVector;
	ECollisionResponse SavedPawnCollisionResponse = ECR_Block;
	FTimerHandle DashCooldownTimer;

	UPROPERTY(VisibleAnywhere, Category = "Seeing")
	TObjectPtr<UPawnSensingComponent> PawnSensingComp;
	UPROPERTY()
	TObjectPtr<AActor> CurrentTarget;

	TMap<TWeakObjectPtr<APawn>, float> DetectedEnemies;

	UPROPERTY(EditDefaultsOnly, Category = "Sensing")
	float TargetLostTime = 1.5f;

	FTimerHandle LostCheckTimer;

	UPROPERTY(EditDefaultsOnly, Category = "Animation|Combat")
	TObjectPtr<UAnimMontage> LeftFireMontage = nullptr;
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Combat")
	TObjectPtr<UAnimMontage> RightFireMontage = nullptr;
	// 사격 몽타주의 재생 속도 배율. 1.0은 원래 속도
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Combat",
	    meta = (ClampMin = "0.01", UIMin = "0.1"))
	float FireMontagePlayRate = 2.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Death")
	TObjectPtr<UAnimSequence> ForwardDeathSequence = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Animation|Death")
	TObjectPtr<UAnimSequence> BackwardDeathSequence = nullptr;
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Animation|Death",
	    meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimSequence> SelectedDeathSequence = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage Feedback", meta = (AllowPrivateAccess = true))
	TSubclassOf<UCameraShakeBase> HitCameraShake;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage Feedback", meta = (AllowPrivateAccess = true))
	float HitStopDuration;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage Feedback", meta = (AllowPrivateAccess = true))
	float HitStopTimeDilation;

	bool bIsMovingSidewaysOrBackward;
	bool bIsSprinting;

	FVector CameraPivotWorldLocation;
	FVector PreviousCameraTargetLocation = FVector::ZeroVector;
	FVector InitialCameraPivotOffset = FVector::ZeroVector;
	bool bDeadZoneWidthInitialized = false;
	float PlayerPreviousHealth;
	FTimerHandle HitStopTimerHandle;
};
