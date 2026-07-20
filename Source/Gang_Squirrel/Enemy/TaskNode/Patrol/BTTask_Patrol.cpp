#include "BTTask_Patrol.h"

#include "AIController.h"
#include "NavigationSystem.h"
#include "Gang_Squirrel/Enemy/GS_Enemy.h"

UBTTask_Patrol::UBTTask_Patrol()
{
	NodeName = TEXT("Patrol");
	bCreateNodeInstance = true;
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_Patrol::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* OwnerAIController = OwnerComp.GetAIOwner();
	AGS_Enemy* Enemy = OwnerAIController ? Cast<AGS_Enemy>(OwnerAIController->GetPawn()) : nullptr;
	
	if (!OwnerAIController || !Enemy)
	{
		return EBTNodeResult::Failed;
	}
	
	PatrolRadius = Enemy->GetEnemyData().PatrolRadius;
	PatrolAcceptanceRadius = Enemy->GetEnemyData().PatrolAcceptanceRadius;
	PatrolRotationInterpSpeed = Enemy->GetEnemyData().PatrolRotationInterpSpeed;
	PatrolWaitTime = Enemy->GetEnemyData().PatrolWaitTime;
	
	UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(OwnerAIController->GetWorld());
	FNavLocation NavLocation;
	if (!NavSystem || !NavSystem->GetRandomReachablePointInRadius(Enemy->GetHomeLocation(), PatrolRadius, NavLocation))
	{
		return EBTNodeResult::Failed;
	}
	
	bReachedPoint = false;
	ElapsedTime = 0.f;
	
	FAIMoveRequest MoveRequest(NavLocation.Location);
	MoveRequest.SetAcceptanceRadius(PatrolAcceptanceRadius);
	const FPathFollowingRequestResult RequestResult = OwnerAIController->MoveTo(MoveRequest);
	
	if (RequestResult.Code == EPathFollowingRequestResult::Failed)
	{
		return EBTNodeResult::Failed;
	}
	
	Enemy->SetRotationTarget(NavLocation.Location, PatrolRotationInterpSpeed);
	CachedAIController = OwnerAIController;
	
	if (RequestResult.Code == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		bReachedPoint = true;
	}
	else
	{
		CachedAIController->ReceiveMoveCompleted.AddDynamic(this,&UBTTask_Patrol::OnMoveCompleted);
	}
	
	
	return EBTNodeResult::InProgress;
}

void UBTTask_Patrol::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);
	
	if (!bReachedPoint)
	{
		return;
	}
	
	ElapsedTime += DeltaSeconds;
	if (ElapsedTime < PatrolWaitTime)
	{
		return;
	}
	
	if (CachedAIController)
	{
		if (AGS_Enemy* Enemy = Cast<AGS_Enemy>(CachedAIController->GetPawn()))
		{
			Enemy->SetRotationTarget(nullptr, PatrolRotationInterpSpeed);
		}
		CachedAIController = nullptr;
	}
	
	FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
}

EBTNodeResult::Type UBTTask_Patrol::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (CachedAIController)
	{
		if (AGS_Enemy* Enemy = Cast<AGS_Enemy>(CachedAIController->GetPawn()))
		{
			Enemy->SetRotationTarget(nullptr,PatrolRotationInterpSpeed);
		}
		CachedAIController->ReceiveMoveCompleted.RemoveDynamic(this, &UBTTask_Patrol::OnMoveCompleted);
		CachedAIController->StopMovement();
		CachedAIController = nullptr;
	}
	
	bReachedPoint = false;
	ElapsedTime = 0.f;
	
	
	return Super::AbortTask(OwnerComp, NodeMemory);
}

void UBTTask_Patrol::OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result)
{
	if (!CachedAIController)
	{
		return;
	}
	
	CachedAIController->ReceiveMoveCompleted.RemoveDynamic(this,&UBTTask_Patrol::OnMoveCompleted);
	
	if (Result == EPathFollowingResult::Success)
	{
		bReachedPoint = true;
		return;
	}
	
	UBehaviorTreeComponent* BTComp =  Cast<UBehaviorTreeComponent>(CachedAIController->GetBrainComponent());
	if (AGS_Enemy* Enemy = Cast<AGS_Enemy>(CachedAIController->GetPawn()))
	{
		Enemy->SetRotationTarget(nullptr,PatrolRotationInterpSpeed);
	}
	CachedAIController = nullptr;
	
	if (BTComp)
	{
		FinishLatentTask(*BTComp,EBTNodeResult::Failed);
	}
		
}
