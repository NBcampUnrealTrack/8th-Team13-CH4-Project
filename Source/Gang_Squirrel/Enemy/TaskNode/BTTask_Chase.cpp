#include "BTTask_Chase.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

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
	
	const float ScaleMultiplier = OwnerAIController->GetPawn() ? OwnerAIController->GetPawn()->GetActorScale3D().Z : 1.f;

	FAIMoveRequest MoveRequest(Target);
	MoveRequest.SetAcceptanceRadius(AcceptanceRadius * ScaleMultiplier);
	
	const FPathFollowingRequestResult RequestResult = OwnerAIController->MoveTo(MoveRequest);
	
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
	
	const FVector ToTarget = (Target->GetActorLocation() - OwnerPawn->GetActorLocation()).GetSafeNormal2D();
	const FRotator DesireRotation = ToTarget.Rotation();
	const FRotator NewRotation = FMath::RInterpTo(OwnerPawn->GetActorRotation(), DesireRotation, DeltaSeconds, RotationInterpSpeed);
	
	OwnerPawn->SetActorRotation(NewRotation);
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
