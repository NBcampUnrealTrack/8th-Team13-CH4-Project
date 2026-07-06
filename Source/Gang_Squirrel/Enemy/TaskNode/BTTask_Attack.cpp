#include "BTTask_Attack.h"

#include "AbilitySystemBlueprintLibrary.h"
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
	bAbilityActivated = false;
	
	if (!CachedEnemy || !CachedTarget)
	{
		return EBTNodeResult::Failed;
	}
	
	FacingToleranceDegrees = CachedEnemy->GetEnemyData().FacingToleranceDegrees;
	RotationInterpSpeed = CachedEnemy->GetEnemyData().AttackRotationInterpSpeed;
	
	if (IsTargetDead(CachedTarget))
	{
		return EBTNodeResult::Failed;
	}
	
	CachedEnemy->SetRotationTarget(CachedTarget,RotationInterpSpeed);
	
	if (IsFacingTarget(CachedEnemy,CachedTarget))
	{
		if (!TryActivateAttack(OwnerComp))
		{
			return EBTNodeResult::Failed;
		}
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
	
	if (IsTargetDead(CachedTarget))
	{
		FinishLatentTask(OwnerComp,EBTNodeResult::Failed);
		return;
	}
	
	if (bAbilityActivated)
	{
		UAbilitySystemComponent* ASC = CachedEnemy->GetAbilitySystemComponent();
		const FGameplayAbilitySpec* Spec = ASC ? ASC->FindAbilitySpecFromClass(CachedEnemy->GetGA_Attack()) : nullptr;
		
		if (!Spec || !Spec->IsActive())
		{
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		}
		return;
	}
	
	if (IsFacingTarget(CachedEnemy,CachedTarget))
	{
		if (!TryActivateAttack(OwnerComp))
		{
			FinishLatentTask(OwnerComp,EBTNodeResult::Failed);
		}
	}
	
}

EBTNodeResult::Type UBTTask_Attack::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (CachedEnemy)
	{
		CachedEnemy->SetRotationTarget(nullptr,RotationInterpSpeed);
	}
	
	CachedEnemy = nullptr;
	CachedTarget = nullptr;
	bAbilityActivated = false;
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

bool UBTTask_Attack::TryActivateAttack(UBehaviorTreeComponent& OwnerComp)
{
	UAbilitySystemComponent* ASC = CachedEnemy->GetAbilitySystemComponent();
	if (!ASC)
	{
		return false;
	}
	
	const bool bHasDeadTag = ASC->HasMatchingGameplayTag(StateTag::TAG_State_Dead);
	const bool bActivated = ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(AbilityTag::TAG_Ability_Attack));

	// UE_LOG(LogGAS, Warning, TEXT("[BTTask_Attack] TryActivateAttack - Enemy:%s, bHasDeadTag:%s, bActivated:%s"), *GetNameSafe(CachedEnemy), bHasDeadTag ? TEXT("true") : TEXT("false"), bActivated ? TEXT("true") : TEXT("false"));

	if (UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent())
	{
		BB->SetValueAsBool(FName("bCanAttack"),false);
	}
	
	bAbilityActivated = bActivated;
	
	if (bAbilityActivated)
	{
		CachedEnemy->SetRotationTarget(nullptr,RotationInterpSpeed);
	}
	
	return bActivated;
}

bool UBTTask_Attack::IsTargetDead(AActor* TargetActor) const
{
	if (!TargetActor)
	{
		return false;
	}
	
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	
	return TargetASC && TargetASC->HasMatchingGameplayTag(StateTag::TAG_State_Dead);
}
