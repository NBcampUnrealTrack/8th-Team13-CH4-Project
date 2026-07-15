#include "AnimNotifyState_DisableBonePhysics.h"

void UAnimNotifyState_DisableBonePhysics::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	
	if (MeshComp && BoneName != NAME_None)
	{
		MeshComp->SetAllBodiesBelowSimulatePhysics(BoneName,false,true);
	}
}

void UAnimNotifyState_DisableBonePhysics::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	
	if (MeshComp && BoneName != NAME_None)
	{
		MeshComp->SetAllBodiesBelowSimulatePhysics(BoneName, true, true);
	}
}
