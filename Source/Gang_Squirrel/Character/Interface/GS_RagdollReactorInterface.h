#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GS_RagdollReactorInterface.generated.h"

UINTERFACE()
class UGS_RagdollReactorInterface : public UInterface
{
	GENERATED_BODY()
};

class GANG_SQUIRREL_API IGS_RagdollReactorInterface
{
	GENERATED_BODY()
public:
	virtual void NetMulticast_ApplyRagdollImpulse(FVector Impulse, FName BoneName) = 0;
	virtual void NetMulticast_SetFullRagdollEnable(bool bEnable) = 0;
	virtual void Applyknockdown(FVector Impulse, FName BoneName, float Duration) = 0;
	virtual FName GetRagdollStartBone() const = 0;
	virtual FVector GetLastHitImpulseDirection() const = 0;
	virtual void SetLastHitImpulseDirection(const FVector& Direction) = 0;
};
