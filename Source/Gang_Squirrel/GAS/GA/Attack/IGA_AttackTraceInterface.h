#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IGA_AttackTraceInterface.generated.h"

UINTERFACE()
class UGA_AttackTraceInterface : public UInterface
{
	GENERATED_BODY()
};

class GANG_SQUIRREL_API IGA_AttackTraceInterface
{
	GENERATED_BODY()
public:
	virtual void OnAttackTraceHit(AActor* HitActor) = 0;
};
