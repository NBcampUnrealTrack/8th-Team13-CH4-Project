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
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;
	
	UPROPERTY(EditAnywhere,Category="BlackBoard|ObjectKey")
	FBlackboardKeySelector TargetActorKey;
	UPROPERTY(EditAnywhere,Category="AI")
	float AcceptanceRadius = 150.f;
	UPROPERTY(EditAnywhere,Category="AI")
	float RotationInterpSpeed = 500.f;
private:
	UFUNCTION()
	void OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result);
	
	UPROPERTY()
	TObjectPtr<AAIController> CachedAIController;
	UPROPERTY()
	TObjectPtr<AActor> CachedMoveTarget;
};
