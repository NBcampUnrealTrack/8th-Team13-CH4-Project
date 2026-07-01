#include "BTTask_Attack.h"

#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Gang_Squirrel/Enemy/GS_Enemy.h"
#include "Gang_Squirrel/GAS/GA/Attack/Enemy/GA_EnemyAttack.h"

UBTTask_Attack::UBTTask_Attack()
{
	NodeName = TEXT("Attack_Target");
}

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* OwnerAIController = OwnerComp.GetAIOwner();
	if (!OwnerAIController)
	{
		return EBTNodeResult::Failed;
	}
	
	AGS_Enemy* Enemy = Cast<AGS_Enemy>(OwnerAIController->GetPawn());
	if (!Enemy)
	{
		return EBTNodeResult::Failed;
	}
	
	UAbilitySystemComponent* ASC = Enemy->GetAbilitySystemComponent();
	if (!ASC || !Enemy->GetGA_Attack())
	{
		return EBTNodeResult::Failed;
	}
	
	const bool bActivated = ASC->TryActivateAbilityByClass(Enemy->GetGA_Attack());
	
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (BB)
	{
		BB->SetValueAsBool(FName("bCanAttack"),false);
	}
	
	return bActivated ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}
