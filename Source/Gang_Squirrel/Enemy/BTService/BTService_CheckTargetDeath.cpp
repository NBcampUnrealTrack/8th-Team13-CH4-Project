#include "BTService_CheckTargetDeath.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Gang_Squirrel/Enemy/GS_Enemy.h"
#include "Gang_Squirrel/GAS/Tags/GS_GamePlayTag.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"
#include "Gang_Squirrel/GAS/GA/Attack/Enemy/GA_EnemyAttack.h"

UBTService_CheckTargetDeath::UBTService_CheckTargetDeath()
{
	NodeName = TEXT("Check_Target_Death");
	Interval = 0.2f;
	RandomDeviation = 0.05f;
	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_CheckTargetDeath,TargetActorKey),AActor::StaticClass());
}

void UBTService_CheckTargetDeath::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	AAIController* AIControllerOwner = OwnerComp.GetAIOwner();
	if (!BB || !AIControllerOwner)
	{
		return;
	}
	
	AGS_Enemy* Enemy = Cast<AGS_Enemy>(AIControllerOwner->GetPawn());
	if (!Enemy)
	{
		return;
	}
	
	AActor* CurrentTarget = Cast<AActor>(BB->GetValueAsObject(TargetActorKey.SelectedKeyName));
	const bool bCurrentTargetDead = CurrentTarget && IsActorDead(CurrentTarget);
	
	if (bCurrentTargetDead)
	{
		AActor* ReplacementTarget = FindNearestTarget(AIControllerOwner, Enemy);
		if (ReplacementTarget)
		{
			BB->SetValueAsObject(TargetActorKey.SelectedKeyName, ReplacementTarget);
			BB->SetValueAsBool(FName("bCanAttack"),false);
			return;
		}

		BB->ClearValue(TargetActorKey.SelectedKeyName);
		BB->SetValueAsBool(FName("bCanAttack"),false);
		Enemy->SetRotationTarget(nullptr, 0.f);
		return;
	}

	const bool bAttacking = IsEnemyAttacking(Enemy);
	if (bAttacking)
	{
		// UE_LOG(LogTemp, Warning, TEXT("[CheckTargetDeath] Skip - Enemy:%s is Attacking"), *GetNameSafe(Enemy));
		return;
	}

	AActor* NearestTarget = FindNearestTarget(AIControllerOwner, Enemy);
	if (!NearestTarget || NearestTarget == CurrentTarget)
	{
		// UE_LOG(LogTemp, Warning, TEXT("[CheckTargetDeath] Enemy:%s, Current:%s, Nearest:%s (no change)"),
		// 	*GetNameSafe(Enemy), *GetNameSafe(CurrentTarget), *GetNameSafe(NearestTarget));
		return;
	}

	if (CurrentTarget)
	{
		const float ScaleMultiplier = Enemy->GetActorScale3D().Z;
		const float ScaledSwitchDistance = SwitchDistance * ScaleMultiplier;

		const float CurrentDist = FVector::Dist(Enemy->GetActorLocation(), CurrentTarget->GetActorLocation());
		const float NearestDist = FVector::Dist(Enemy->GetActorLocation(), NearestTarget->GetActorLocation());

		// UE_LOG(LogTemp, Warning, TEXT("[CheckTargetDeath] Enemy:%s, Current:%s(%.1f), Nearest:%s(%.1f), Diff:%.1f, Threshold:%.1f"),
		// 	*GetNameSafe(Enemy), *GetNameSafe(CurrentTarget), CurrentDist, *GetNameSafe(NearestTarget), NearestDist, CurrentDist - NearestDist, ScaledSwitchDistance);

		if (CurrentDist - NearestDist < ScaledSwitchDistance)
		{
			return;
		}
	}

	// UE_LOG(LogTemp, Warning, TEXT("[CheckTargetDeath] SWITCH! Enemy:%s -> NewTarget:%s"), *GetNameSafe(Enemy), *GetNameSafe(NearestTarget));

	BB->SetValueAsObject(TargetActorKey.SelectedKeyName, NearestTarget);
	BB->SetValueAsBool(FName("bCanAttack"),false);
}

bool UBTService_CheckTargetDeath::IsActorDead(AActor* Actor) const
{
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor);
	
	return ASC && ASC->HasMatchingGameplayTag(StateTag::TAG_State_Dead);
}

bool UBTService_CheckTargetDeath::IsEnemyAttacking(AGS_Enemy* Enemy) const
{
	UAbilitySystemComponent* ASC = Enemy->GetAbilitySystemComponent();
	const FGameplayAbilitySpec* Spec = ASC ? ASC->FindAbilitySpecFromClass(Enemy->GetGA_Attack()) : nullptr;
	
	return Spec && Spec->IsActive();
}

AActor* UBTService_CheckTargetDeath::FindNearestTarget(AAIController* AIController, AActor* SelfActor) const
{
	UAIPerceptionComponent* PerceptionComp = AIController->GetPerceptionComponent();
	
	if (!PerceptionComp)
	{
		return nullptr;
	}
	
	TArray<AActor*> PerceivedActors;
	PerceptionComp->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), PerceivedActors);

	AActor* NearestActor = nullptr;
	float NearestDistSq = TNumericLimits<float>::Max();

	for (AActor* Candidate : PerceivedActors)
	{
		if (!Candidate || Candidate == SelfActor || IsActorDead(Candidate))
		{
			continue;
		}

		UAbilitySystemComponent* CandidateASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Candidate);
		if (!CandidateASC || CandidateASC->HasMatchingGameplayTag(TeamTag::TAG_Team_Enemy))
		{
			continue;
		}

		const float Distsq = FVector::DistSquared(SelfActor->GetActorLocation(), Candidate->GetActorLocation());
		if (Distsq < NearestDistSq)
		{
			NearestDistSq = Distsq;
			NearestActor = Candidate;
		}
	}

	return NearestActor;
}
