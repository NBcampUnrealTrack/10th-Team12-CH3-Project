#include "AIBaseController.h"
#include "AIBaseCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AISense.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Sound/SoundBase.h"


AAIBaseController::AAIBaseController()
{
	// 시야 감지 설정
	AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
	SetPerceptionComponent(*AIPerception);

	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = 1500.0f;
	SightConfig->LoseSightRadius = 2000.0f;
	SightConfig->PeripheralVisionAngleDegrees = 90.0f;
	SightConfig->SetMaxAge(5.0f);

	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

	AIPerception->ConfigureSense(*SightConfig);
	AIPerception->SetDominantSense(SightConfig->GetSenseImplementation());
}

void AAIBaseController::StartBehaviorTree()
{
	if (BehaviorTreeAsset)
	{
		RunBehaviorTree(BehaviorTreeAsset);
	}
}

void AAIBaseController::BeginPlay()
{
	Super::BeginPlay();

	if (AIPerception)
	{
		AIPerception->OnTargetPerceptionUpdated.AddDynamic(this, &AAIBaseController::OnPerceptionUpdated);
	}

	StartBehaviorTree();
}

void AAIBaseController::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (Actor != PlayerPawn)
	{
		return;
	}

	UBlackboardComponent* BlackboardComp = GetBlackboardComponent();
	if (!BlackboardComp)
	{
		return;
	}

	// =========================
	// Sight
	// =========================

	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			PlayAIPerceptionSound(DetectbySightSound);

			GetWorld()->GetTimerManager().ClearTimer(LoseSightTimer);

			BlackboardComp->SetValueAsObject(TEXT("TargetActor"), Actor);

			BlackboardComp->SetValueAsBool(TEXT("IsChasing"), true);

			BlackboardComp->SetValueAsVector(TEXT("LastKnownLocation"), Actor->GetActorLocation());

			if (AAIBaseCharacter* AICharacter = Cast<AAIBaseCharacter>(GetPawn()))
			{
				AICharacter->SetMovementSpeed(AICharacter->RunSpeed);
				AICharacter->SetAlertChasing(true);
			}
		}
		else
		{
			// 시야에서 놓침
			GetWorld()->GetTimerManager().SetTimer(
				LoseSightTimer,
				this,
				&AAIBaseController::StopChasing,
				LoseSightDelay,
				false);
		}
	}
	else if (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
	{
		// =========================
		// Hearing
		// =========================
		if (Stimulus.WasSuccessfullySensed())
		{
			BlackboardComp->SetValueAsVector(TEXT("LastKnownLocation"), Stimulus.StimulusLocation);
		}
	}
}

void AAIBaseController::PlayAIPerceptionSound(USoundBase* SoundToPlay) const
{
	if (!SoundToPlay)
	{
		return;
	}

	const APawn* thisPawn = GetPawn();

	if (!thisPawn)
	{
		return;
	}

	UGameplayStatics::PlaySoundAtLocation(
		this,
		SoundToPlay,
		thisPawn->GetActorLocation(),
		1.0f,
		1.0f,
		0.0f,
		AISoundAttenuation);
}

void AAIBaseController::StopChasing()
{
	UBlackboardComponent* BlackboardComp = GetBlackboardComponent();

	if (!BlackboardComp)
	{
		return;
	}

	BlackboardComp->ClearValue(TEXT("TargetActor"));

	BlackboardComp->SetValueAsBool(TEXT("IsChasing"), false);

	if (AAIBaseCharacter* AICharacter = Cast<AAIBaseCharacter>(GetPawn()))
	{
		AICharacter->SetMovementSpeed(AICharacter->WalkSpeed);
		AICharacter->SetAlertChasing(false);
	}
}
