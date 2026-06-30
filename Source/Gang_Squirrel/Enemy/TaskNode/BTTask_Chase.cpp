#include "BTTask_Chase.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_Chase::UBTTask_Chase()
{
	NodeName = TEXT("Chase_Target");
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
	
	FAIRequestID RequestID = OwnerAIController->MoveToActor(Target,AcceptanceRadius);
	
	if (RequestID.IsValid())
	{
		WaitForMessage(OwnerComp, UBrainComponent::AIMessage_MoveFinished,RequestID);
		
		return EBTNodeResult::InProgress;
	}
	
	return EBTNodeResult::Failed;
}

void UBTTask_Chase::OnMessage(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, FName Message, int32 RequestID,
	bool bSuccess)
{
	Super::OnMessage(OwnerComp, NodeMemory, Message, RequestID, bSuccess);
	
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	
	if (bSuccess && BB)
	{
		// When Enemy Reached to TargetActor
		BB->SetValueAsBool(FName("bCanAttack"),true);
		FinishLatentTask(OwnerComp,EBTNodeResult::Succeeded);
	}
	else
	{
		FinishLatentTask(OwnerComp,EBTNodeResult::Failed);
	}
	
}
