#include "BTService_CheckTargetDeath.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Gang_Squirrel/Enemy/GS_Enemy.h"
#include "Gang_Squirrel/GAS/Tags/GS_GamePlayTag.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"

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
	
	AActor* CurrentTarget = Cast<AActor>(BB->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (!CurrentTarget || !IsActorDead(CurrentTarget))
	{
		return;
	}
	
	BB->ClearValue(TargetActorKey.SelectedKeyName);
	BB->SetValueAsBool(FName("bCanAttack"),false);
	
	if (AGS_Enemy* Enemy = Cast<AGS_Enemy>(AIControllerOwner->GetPawn()))
	{
		Enemy->SetRotationTarget(nullptr,0.f);
	}
	
	if (AActor* NewTarget = FindAliveTarget(AIControllerOwner, AIControllerOwner->GetPawn()))
	{
		BB->SetValueAsObject(TargetActorKey.SelectedKeyName,NewTarget);
	}
		
	
}

bool UBTService_CheckTargetDeath::IsActorDead(AActor* Actor) const
{
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor);
	
	return ASC && ASC->HasMatchingGameplayTag(StateTag::TAG_State_Dead);
}

AActor* UBTService_CheckTargetDeath::FindAliveTarget(AAIController* AIController, AActor* SelfActor) const
{
	UAIPerceptionComponent* PerceptionComp = AIController->GetPerceptionComponent();
	
	if (!PerceptionComp)
	{
		return nullptr;
	}
	
	TArray<AActor*> PerceivedActors;
	PerceptionComp->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), PerceivedActors);
	
	for (AActor* Candidate : PerceivedActors)
	{
		if (!Candidate || Candidate == SelfActor || IsActorDead(Candidate))
		{
			continue;
		}
		
		UAbilitySystemComponent* CandidateASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Candidate);
		if (CandidateASC && CandidateASC->HasMatchingGameplayTag(TeamTag::TAG_Team_Player))
		{
			return Candidate;
		}
	}
	
	return nullptr;
}
