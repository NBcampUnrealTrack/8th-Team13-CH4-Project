#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_AttackTrace.generated.h"

UCLASS()
class GANG_SQUIRREL_API UAnimNotifyState_AttackTrace : public UAnimNotifyState
{
	GENERATED_BODY()
	
public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	
	UPROPERTY(EditAnywhere,Category="AttackTrace")
	FName TraceSocketName = NAME_None;
	UPROPERTY(EditAnywhere,Category="AttackTrace")
	float TraceRadius = 32.f;
	UPROPERTY(EditAnywhere,Category="AttackTrace")
	bool bDrawDebug = true;
	
private:
	TMap<TWeakObjectPtr<USkeletalMeshComponent>,FVector> PrevSocketLocationMap;
};
