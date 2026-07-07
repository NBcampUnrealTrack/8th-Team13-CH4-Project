#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_CheckTargetDeath.generated.h"

UCLASS()
class GANG_SQUIRREL_API UBTService_CheckTargetDeath : public UBTService
{
	GENERATED_BODY()
	
public:
	UBTService_CheckTargetDeath();
	
protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
	UPROPERTY(EditAnywhere,Category="BlackBoard|ObjectKey")
	FBlackboardKeySelector TargetActorKey;
	
private:
	bool IsActorDead(AActor* Actor) const;
	AActor* FindAliveTarget(AAIController* AIController, AActor* SelfActor) const;
};
