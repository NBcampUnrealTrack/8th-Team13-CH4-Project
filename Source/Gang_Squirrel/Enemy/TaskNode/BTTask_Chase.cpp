#include "BTTask_Chase.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_Chase::UBTTask_Chase()
{
	NodeName = TEXT("Chase_Target");
	bCreateNodeInstance = true;
	TargetActorKey.AddObjectFilter(this,GET_MEMBER_NAME_CHECKED(UBTTask_Chase,TargetActorKey),AActor::StaticClass());
}

EBTNodeResult::Type UBTTask_Chase::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* OwnerAIController = OwnerComp.GetAIOwner();
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	
	if (!OwnerAIController || !BB)
	{
		return EBTNodeResult::Failed;
	}
	
	AActor* Target = Cast<AActor>(BB->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (!Target)
	{
		return EBTNodeResult::Failed;
	}
	
	FAIRequestID NewRequestID = OwnerAIController->MoveToActor(Target,AcceptanceRadius);
	
	UE_LOG(LogTemp, Warning, TEXT("[Chase] ExecuteTask - RequestID: %u, Valid:%s"),(uint32)NewRequestID, NewRequestID.IsValid() ? TEXT("true") :TEXT("false"));
	
	if (!NewRequestID.IsValid())
	{
		BB->SetValueAsBool(FName("bCanAttack"),true);
		return EBTNodeResult::Succeeded;
	}
	
	CachedAIController = OwnerAIController;
	CachedAIController->ReceiveMoveCompleted.AddDynamic(this,&UBTTask_Chase::OnMoveCompleted);
	
	return EBTNodeResult::InProgress;
}

// When Task Wask Aborted
EBTNodeResult::Type UBTTask_Chase::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (CachedAIController)
	{
		CachedAIController->ReceiveMoveCompleted.RemoveDynamic(this, &UBTTask_Chase::OnMoveCompleted);
		CachedAIController->StopMovement();
		CachedAIController = nullptr;
	}
	
	return Super::AbortTask(OwnerComp, NodeMemory);
}

void UBTTask_Chase::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	if (CachedAIController)
	{
		CachedAIController->ReceiveMoveCompleted.RemoveDynamic(this,&UBTTask_Chase::OnMoveCompleted);
		CachedAIController = nullptr;
	}
	
	
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

void UBTTask_Chase::OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result)
{
	if (!CachedAIController)
	{
		return;
	}
	
	UBehaviorTreeComponent* BTComp = Cast<UBehaviorTreeComponent>(CachedAIController->GetBrainComponent());
	
	if (!BTComp)
	{
		CachedAIController->ReceiveMoveCompleted.RemoveDynamic(this,&UBTTask_Chase::OnMoveCompleted);
		CachedAIController = nullptr;
		return;
	}
	
	CachedAIController->ReceiveMoveCompleted.RemoveDynamic(this,&UBTTask_Chase::OnMoveCompleted);
	CachedAIController = nullptr;
	
	if (Result == EPathFollowingResult::Success)
	{
		UBlackboardComponent* BB = BTComp->GetBlackboardComponent();
		if (BB)
		{
			BB->SetValueAsBool(FName("bCanAttack"),true);
		}
		FinishLatentTask(*BTComp,EBTNodeResult::Succeeded);
	}
	else
	{
		FinishLatentTask(*BTComp, EBTNodeResult::Failed);
	}
} 
