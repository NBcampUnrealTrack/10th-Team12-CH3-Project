#include "AnimNotify_BossAttackHit.h"

#include "AIBossCharacter.h"
#include "Components/SkeletalMeshComponent.h"

void UAnimNotify_BossAttackHit::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp)
	{
		return;
	}

	AAIBossCharacter* BossCharacter = Cast<AAIBossCharacter>(MeshComp->GetOwner());

	if (!BossCharacter)
	{
		return;
	}

	BossCharacter->ApplyCurrentPatternDamage();
}