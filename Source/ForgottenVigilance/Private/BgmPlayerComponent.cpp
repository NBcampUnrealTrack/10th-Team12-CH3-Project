#include "BgmPlayerComponent.h"

#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"

namespace
{
constexpr float DefaultFadeDuration = 1.5f;
constexpr float DefaultMasterVolume = 1.0f;
constexpr float BgmZeroVolume = 0.0f;
}

UBgmPlayerComponent::UBgmPlayerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	FadeDuration = DefaultFadeDuration;
	bContinueWhenPaused = true;
	ChannelA = nullptr;
	ChannelB = nullptr;
	CurrentTrack = nullptr;
	MasterVolume = DefaultMasterVolume;
	bUsingChannelA = false;
}

void UBgmPlayerComponent::BeginPlay()
{
	Super::BeginPlay();

	ChannelA = CreateChannel(TEXT("BgmChannelA"));
	ChannelB = CreateChannel(TEXT("BgmChannelB"));

	AForgottenGameState* ForgottenGameState = Cast<AForgottenGameState>(GetOwner());

	if (!ForgottenGameState)
	{
		return;
	}

	ForgottenGameState->OnPhaseChanged.AddDynamic(this, &UBgmPlayerComponent::HandlePhaseChanged);

	float LevelVolume = DefaultMasterVolume;
	USoundBase* LevelTrack = FindTrackForLevel(LevelVolume);

	if (LevelTrack)
	{
		PlayTrack(LevelTrack, LevelVolume);
		return;
	}

	PlayTrackForPhase(ForgottenGameState->GetPhase());
}

USoundBase* UBgmPlayerComponent::FindTrackForLevel(float& OutVolume) const
{
	const FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(this, true);

	for (const FLevelBgmTrack& LevelTrack : LevelTracks)
	{
		if (LevelTrack.LevelName.IsNone() || !LevelTrack.Track)
		{
			continue;
		}

		if (!CurrentLevelName.Contains(LevelTrack.LevelName.ToString()))
		{
			continue;
		}

		OutVolume = LevelTrack.Volume;

		return LevelTrack.Track;
	}

	return nullptr;
}

UAudioComponent* UBgmPlayerComponent::CreateChannel(const FName& ChannelName)
{
	AActor* OwnerActor = GetOwner();

	if (!OwnerActor)
	{
		return nullptr;
	}

	UAudioComponent* Channel = NewObject<UAudioComponent>(OwnerActor, ChannelName);

	if (!Channel)
	{
		return nullptr;
	}

	Channel->bAutoActivate = false;
	Channel->bAutoDestroy = false;
	Channel->bAllowSpatialization = false;
	Channel->bIsUISound = bContinueWhenPaused;
	Channel->RegisterComponent();

	return Channel;
}

void UBgmPlayerComponent::HandlePhaseChanged(EForgottenPhase NewPhase)
{
	PlayTrackForPhase(NewPhase);
}

void UBgmPlayerComponent::PlayTrackForPhase(EForgottenPhase Phase)
{
	float TrackVolume = DefaultMasterVolume;
	USoundBase* PhaseTrack = FindTrackForPhase(Phase, TrackVolume);

	if (!PhaseTrack)
	{
		return;
	}

	PlayTrack(PhaseTrack, TrackVolume);
}

USoundBase* UBgmPlayerComponent::FindTrackForPhase(EForgottenPhase Phase, float& OutVolume) const
{
	for (const FBgmTrack& PhaseTrack : PhaseTracks)
	{
		if (PhaseTrack.Phase != Phase || !PhaseTrack.Track)
		{
			continue;
		}

		OutVolume = PhaseTrack.Volume;

		return PhaseTrack.Track;
	}

	return nullptr;
}

void UBgmPlayerComponent::PlayTrack(USoundBase* NewTrack, float TrackVolume)
{
	if (!NewTrack || NewTrack == CurrentTrack)
	{
		return;
	}

	if (!ChannelA || !ChannelB)
	{
		return;
	}

	UAudioComponent* OutgoingChannel = bUsingChannelA ? ChannelA : ChannelB;
	UAudioComponent* IncomingChannel = bUsingChannelA ? ChannelB : ChannelA;

	if (OutgoingChannel->IsPlaying())
	{
		OutgoingChannel->FadeOut(FadeDuration, BgmZeroVolume);
	}

	IncomingChannel->SetSound(NewTrack);
	IncomingChannel->FadeIn(FadeDuration, TrackVolume * MasterVolume);

	CurrentTrack = NewTrack;
	bUsingChannelA = !bUsingChannelA;
}

void UBgmPlayerComponent::StopTrack()
{
	if (ChannelA && ChannelA->IsPlaying())
	{
		ChannelA->FadeOut(FadeDuration, BgmZeroVolume);
	}

	if (ChannelB && ChannelB->IsPlaying())
	{
		ChannelB->FadeOut(FadeDuration, BgmZeroVolume);
	}

	CurrentTrack = nullptr;
}

void UBgmPlayerComponent::SetMasterVolume(float NewVolume)
{
	MasterVolume = FMath::Clamp(NewVolume, BgmZeroVolume, DefaultMasterVolume);

	UAudioComponent* ActiveChannel = bUsingChannelA ? ChannelB : ChannelA;

	if (!ActiveChannel)
	{
		return;
	}

	ActiveChannel->SetVolumeMultiplier(MasterVolume);
}

float UBgmPlayerComponent::GetMasterVolume() const
{
	return MasterVolume;
}
