#include "BTTask_Attack.h"

#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Gang_Squirrel/Enemy/GS_Enemy.h"
#include "Gang_Squirrel/GAS/GA/Attack/Enemy/GA_EnemyAttack.h"
#include "Gang_Squirrel/Gang_Squirrel.h"
#include "Gang_Squirrel/Character/GSCharacter.h"
#include "Gang_Squirrel/GAS/Tags/GS_GamePlayTag.h"

UBTTask_Attack::UBTTask_Attack()
{
	NodeName = TEXT("Attack_Target");
	bNotifyTick = true;
	bCreateNodeInstance = true;
	TargetActorkey.AddObjectFilter(this,GET_MEMBER_NAME_CHECKED(UBTTask_Attack,TargetActorkey),AActor::StaticClass());
}

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* OwnerAIController = OwnerComp.GetAIOwner();
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!OwnerAIController || !BB)
	{
		return EBTNodeResult::Failed;
	}
	
	CachedEnemy = Cast<AGS_Enemy>(OwnerAIController->GetPawn());
	CachedTarget = Cast<AGSCharacter>(BB->GetValueAsObject(TargetActorkey.SelectedKeyName));
	
	if (!CachedEnemy || !CachedTarget)
	{
		return EBTNodeResult::Failed;
	}
	
	if (IsFacingTarget(CachedEnemy,CachedTarget))
	{
		return TryActivateAttack(OwnerComp);
	}
	
	return EBTNodeResult::InProgress;
}

void UBTTask_Attack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);
	
	if (!CachedEnemy || !CachedTarget)
	{
		FinishLatentTask(OwnerComp,EBTNodeResult::Failed);
		return;
	}
	
	const FVector ToTarget = (CachedTarget->GetActorLocation() - CachedEnemy->GetActorLocation()).GetSafeNormal2D();
	const FRotator DesiredRotation = ToTarget.Rotation();
	const FRotator NewRotation = FMath::RInterpTo(CachedEnemy->GetActorRotation(),DesiredRotation,DeltaSeconds,RotationInterpSpeed);
	
	CachedEnemy->SetActorRotation(NewRotation);
	
	if (IsFacingTarget(CachedEnemy,CachedTarget))
	{
		FinishLatentTask(OwnerComp,TryActivateAttack(OwnerComp));
	}
	
}

EBTNodeResult::Type UBTTask_Attack::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	CachedEnemy = nullptr;
	CachedTarget = nullptr;
	return Super::AbortTask(OwnerComp, NodeMemory);
}

bool UBTTask_Attack::IsFacingTarget(const AActor* Enemy, const AActor* Target) const
{
	const FVector ToTarget = (Target->GetActorLocation() - Enemy->GetActorLocation()).GetSafeNormal2D();
	const FVector Forward = Enemy->GetActorForwardVector().GetSafeNormal2D();
	
	const float Dot = FVector::DotProduct(Forward,ToTarget);
	const float AngleDegrees = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(Dot,-1.f,1.f)));
	
	return AngleDegrees <= FacingToleranceDegrees;
}

EBTNodeResult::Type UBTTask_Attack::TryActivateAttack(UBehaviorTreeComponent& OwnerComp)
{
	UAbilitySystemComponent* ASC = CachedEnemy->GetAbilitySystemComponent();
	if (!ASC)
	{
		return EBTNodeResult::Failed;
	}
	
	const bool bActivated = ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(AbilityTag::TAG_Ability_Attack));
	
	if (UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent())
	{
		BB->SetValueAsBool(FName("bCanAttack"),false);
	}
	
	return bActivated ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}
