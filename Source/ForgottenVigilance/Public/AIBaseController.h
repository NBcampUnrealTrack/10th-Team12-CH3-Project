// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Perception/AIPerceptionTypes.h"
#include "AIBaseController.generated.h"

class UAIPerceptionComponent;
class UAISenseConfig_Sight;

UCLASS()
class FORGOTTENVIGILANCE_API AAIBaseController : public AAIController
{
	GENERATED_BODY()

public:
	AAIBaseController();

	void StartBehaviorTree();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	class UBehaviorTree* BehaviorTreeAsset;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UAIPerceptionComponent> AIPerception;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Sound", meta = (AllowPrivateAccess = true))
	TObjectPtr<USoundBase> DetectbySightSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Sound", meta = (AllowPrivateAccess = true))
	TObjectPtr<USoundAttenuation> AISoundAttenuation;

	UPROPERTY(EditAnywhere, Category = "AI")
	float LoseSightDelay = 10.0f;

	virtual void BeginPlay() override;

	FTimerHandle LoseSightTimer;

	void PlayAIPerceptionSound(USoundBase* SoundToPlay) const;

	void StopChasing();

	UFUNCTION()
	void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);


};
