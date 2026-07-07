#include "BTTask_Chase.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Gang_Squirrel/Enemy/GS_Enemy.h"

UBTTask_Chase::UBTTask_Chase()
{
	NodeName = TEXT("Chase_Target");
	bCreateNodeInstance = true;
	bNotifyTick = true;
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
	if (AGS_Enemy* Enemy = Cast<AGS_Enemy>(OwnerAIController->GetPawn()))
	{
		AcceptanceRadius = Enemy->GetEnemyData().AcceptanceRadius;
		RotationInterpSpeed = Enemy->GetEnemyData().ChaseRotationInterpSpeed;
	}
	
	FAIMoveRequest MoveRequest(Target);
	MoveRequest.SetAcceptanceRadius(AcceptanceRadius);
	
	const FPathFollowingRequestResult RequestResult = OwnerAIController->MoveTo(MoveRequest);
	CachedMoveTarget = Target;
	
	if (RequestResult.Code == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		BB->SetValueAsBool(FName("bCanAttack"),true);
		return EBTNodeResult::Succeeded;
	}
	
	if (RequestResult.Code != EPathFollowingRequestResult::RequestSuccessful)
	{
		return EBTNodeResult::Failed;
	}
	
	CachedAIController = OwnerAIController;
	CachedAIController->ReceiveMoveCompleted.AddDynamic(this,&UBTTask_Chase::OnMoveCompleted);
	
	return EBTNodeResult::InProgress;
}

void UBTTask_Chase::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);
	
	if (!CachedAIController)
	{
		return;
	}
	
	APawn* OwnerPawn = CachedAIController->GetPawn();
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!OwnerPawn || !BB)
	{
		return;
	}
	
	AActor* Target = Cast<AActor>(BB->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (!Target)
	{
		return;
	}
	
	if (Target != CachedMoveTarget)
	{
		FAIMoveRequest MoveRequest(Target);
		MoveRequest.SetAcceptanceRadius(AcceptanceRadius);
		CachedAIController->MoveTo(MoveRequest);
		CachedMoveTarget = Target;
	}
	
	if (AGS_Enemy* Enemy = Cast<AGS_Enemy>(OwnerPawn))
	{
		Enemy->SetRotationTarget(Target,RotationInterpSpeed);
	}
}

// When Task Wask Aborted
EBTNodeResult::Type UBTTask_Chase::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (CachedAIController)
	{
		if (AGS_Enemy* Enemy = Cast<AGS_Enemy>(CachedAIController->GetPawn()))
		{
			Enemy->SetRotationTarget(nullptr,RotationInterpSpeed);
		}
		CachedAIController->ReceiveMoveCompleted.RemoveDynamic(this, &UBTTask_Chase::OnMoveCompleted);
		CachedAIController->StopMovement();
		CachedAIController = nullptr;
		CachedMoveTarget = nullptr;
	}
	
	return Super::AbortTask(OwnerComp, NodeMemory);
}

void UBTTask_Chase::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	if (CachedAIController)
	{
		if (AGS_Enemy* Enemy = Cast<AGS_Enemy>(CachedAIController->GetPawn()))
		{
			Enemy->SetRotationTarget(nullptr,RotationInterpSpeed);
		}
		CachedAIController->ReceiveMoveCompleted.RemoveDynamic(this,&UBTTask_Chase::OnMoveCompleted);
		CachedAIController = nullptr;
		CachedMoveTarget = nullptr;
	}
	
	
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

void UBTTask_Chase::OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result)
{
	UE_LOG(LogTemp, Warning, TEXT("[Chase] OnMoveCompleted - RequestID: %u, Result: %d"),(uint32)RequestID, (int32)Result);
	
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
