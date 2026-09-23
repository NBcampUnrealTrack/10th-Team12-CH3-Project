#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ForgottenGameState.h"
#include "BgmPlayerComponent.generated.h"

class UAudioComponent;
class USoundBase;

USTRUCT(BlueprintType)
struct FBgmTrack
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BGM")
	EForgottenPhase Phase = EForgottenPhase::Exploring;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BGM")
	TObjectPtr<USoundBase> Track = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BGM")
	float Volume = 1.0f;
};

USTRUCT(BlueprintType)
struct FLevelBgmTrack
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BGM")
	FName LevelName = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BGM")
	TObjectPtr<USoundBase> Track = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BGM")
	float Volume = 1.0f;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class FORGOTTENVIGILANCE_API UBgmPlayerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBgmPlayerComponent();

	void PlayTrack(USoundBase* NewTrack, float TrackVolume);
	void StopTrack();
	void SetMasterVolume(float NewVolume);
	float GetMasterVolume() const;

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void HandlePhaseChanged(EForgottenPhase NewPhase);

	UAudioComponent* CreateChannel(const FName& ChannelName);
	USoundBase* FindTrackForPhase(EForgottenPhase Phase, float& OutVolume) const;
	void PlayTrackForPhase(EForgottenPhase Phase);
	USoundBase* FindTrackForLevel(float& OutVolume) const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BGM", meta = (AllowPrivateAccess = true))
	TArray<FLevelBgmTrack> LevelTracks;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BGM", meta = (AllowPrivateAccess = true))
	TArray<FBgmTrack> PhaseTracks;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BGM", meta = (AllowPrivateAccess = true))
	float FadeDuration;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BGM", meta = (AllowPrivateAccess = true))
	bool bContinueWhenPaused;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BGM", meta = (AllowPrivateAccess = true))
	TObjectPtr<UAudioComponent> ChannelA;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BGM", meta = (AllowPrivateAccess = true))
	TObjectPtr<UAudioComponent> ChannelB;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BGM", meta = (AllowPrivateAccess = true))
	TObjectPtr<USoundBase> CurrentTrack;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BGM", meta = (AllowPrivateAccess = true))
	float MasterVolume;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BGM", meta = (AllowPrivateAccess = true))
	bool bUsingChannelA;
};
