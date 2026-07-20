#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Navigation/PathFollowingComponent.h"
#include "BTTask_Patrol.generated.h"

UCLASS()
class GANG_SQUIRREL_API UBTTask_Patrol : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_Patrol();
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	UPROPERTY(EditAnywhere,Category="AI")
	float PatrolRadius = 500.f;
	UPROPERTY(EditAnywhere,Category="AI")
	float PatrolAcceptanceRadius = 30.f;
	UPROPERTY(EditAnywhere,Category="AI")
	float PatrolRotationInterpSpeed = 20.f;
	UPROPERTY(EditAnywhere,Category="AI")
	float PatrolWaitTime = 2.f;
	
private:
	UFUNCTION()
	void OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result);
	UPROPERTY()
	TObjectPtr<AAIController> CachedAIController;
	
	bool bReachedPoint = false;
	float ElapsedTime = 0.f;
	
};
