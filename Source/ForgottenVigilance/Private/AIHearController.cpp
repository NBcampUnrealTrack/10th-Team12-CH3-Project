#include "AIHearController.h"

#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AIPerceptionComponent.h"

AAIHearController::AAIHearController()
{
	HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));

	HearingConfig->HearingRange = 3000.0f;

	HearingConfig->SetMaxAge(5.0f);

	HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
	HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
	HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;

	AIPerception->ConfigureSense(*HearingConfig);
}
