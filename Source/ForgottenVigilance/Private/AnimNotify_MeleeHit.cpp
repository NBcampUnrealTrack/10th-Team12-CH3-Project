#include "AnimNotify_MeleeHit.h"
#include "MeleeAttackComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

void UAnimNotify_MeleeHit::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp)
	{
		return;
	}

	AActor* OwnerActor = MeshComp->GetOwner();

	if (!OwnerActor)
	{
		return;
	}

	if (UMeleeAttackComponent* MeleeAttackComponent = OwnerActor->FindComponentByClass<UMeleeAttackComponent>())
	{
		MeleeAttackComponent->ApplyCachedDamage();
	}
}
