#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Attack.generated.h"

class AGS_Enemy;

UCLASS()
class GANG_SQUIRREL_API UBTTask_Attack : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_Attack();
	
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	UPROPERTY(EditAnywhere,Category="BlackBoard|ObjectKey")
	FBlackboardKeySelector TargetActorkey;
	
	//TODO::Refac to DataTable
	UPROPERTY(EditAnywhere,Category="AI")
	float FacingToleranceDegrees = 10.f;
	UPROPERTY(EditAnywhere,Category="AI")
	float RotationInterpSpeed = 8.f;
	
private:
	bool IsFacingTarget(const AActor* Enemy, const AActor* Target)const;
	bool TryActivateAttack(UBehaviorTreeComponent& OwnerComp);
	
	UPROPERTY()
	TObjectPtr<AGS_Enemy> CachedEnemy;
	UPROPERTY()
	TObjectPtr<AActor> CachedTarget;
	
	bool bAbilityActivated = false;
};
