#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Navigation/PathFollowingComponent.h"
#include "BTTask_Chase.generated.h"

UCLASS()
class GANG_SQUIRREL_API UBTTask_Chase : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTTask_Chase();
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;
	
	UPROPERTY(EditAnywhere,Category="BlackBoard|ObjectKey")
	FBlackboardKeySelector TargetActorKey;
	// TODO::Refac to DataTable
	UPROPERTY(EditAnywhere,Category="AI")
	float AcceptanceRadius = 150.f;
private:
	UFUNCTION()
	void OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result);
	
	UPROPERTY()
	TObjectPtr<AAIController> CachedAIController;
};
