#include "GS_EnemyAIController.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Gang_Squirrel/Enemy/GS_Enemy.h"
#include "Gang_Squirrel/GAS/GA/GA_AbilityBase.h"
#include "Gang_Squirrel/GAS/Tags/GS_GamePlayTag.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AIPerceptionTypes.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Sight.h"


AGS_EnemyAIController::AGS_EnemyAIController()
{
	PrimaryActorTick.bCanEverTick = true;
	
	//TODO::DataTable,Asset Refac
	PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
	
#pragma region SightConfig_Settings
	Sight_Config = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	Sight_Config->SightRadius = BaseSightRadius;
	Sight_Config->LoseSightRadius = BaseLoseSightRadius;
	// *2 = SightDegrees is 360
	Sight_Config->PeripheralVisionAngleDegrees = 180.f;
	Sight_Config->SetMaxAge(5.f);
	// Detection Type
	Sight_Config->DetectionByAffiliation.bDetectEnemies = true;
	Sight_Config->DetectionByAffiliation.bDetectFriendlies = true;
	Sight_Config->DetectionByAffiliation.bDetectNeutrals = true;
#pragma endregion 
	
#pragma region PerceptionCompSettings
	PerceptionComp->ConfigureSense(*Sight_Config);
	PerceptionComp->SetDominantSense(UAISenseConfig_Sight::StaticClass());
	PerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this,&AGS_EnemyAIController::OnTargetPerceptionUpdated);
	
	SetPerceptionComponent(*PerceptionComp);
#pragma endregion
}

//For Debugging Sight Radius
void AGS_EnemyAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	// DrawDebug_SightRadius();
}

void AGS_EnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	if (const AGS_Enemy* Enemy = Cast<AGS_Enemy>(InPawn))
	{
		Sight_Config->SightRadius = Enemy->GetEnemyData().SightRadius;
		Sight_Config->LoseSightRadius = Enemy->GetEnemyData().LoseSightRadius;
		PerceptionComp->ConfigureSense(*Sight_Config);
	}

	if (BT_Enemy)
	{
		RunBehaviorTree(BT_Enemy);
	}
}

void AGS_EnemyAIController::RestartBehaviorTree()
{
	if (BT_Enemy)
	{
		RunBehaviorTree(BT_Enemy);
	}
}

// When Sight Sense Updated(Lose,Detection...)
void AGS_EnemyAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	UBlackboardComponent* BB = GetBlackboardComponent();
	if (!BB)
	{
		return;
	}

	if (UGA_AbilityBase::IsSameTeam(GetPawn(), Actor))
	{
		return;
	}

	/*const float Distance = GetPawn() ? FVector::Dist(GetPawn()->GetActorLocation(), Stimulus.StimulusLocation) : -1.f;
	UE_LOG(LogTemp, Warning, TEXT("[Perception] Actor:%s, WasSuccessfullySensed:%s, Distance:%.1f, SightRadius:%.1f, LoseSightRadius:%.1f"),
		*GetNameSafe(Actor),
		Stimulus.WasSuccessfullySensed() ? TEXT("true") : TEXT("false"),
		Distance,
		Sight_Config ? Sight_Config->SightRadius : -1.f,
		Sight_Config ? Sight_Config->LoseSightRadius : -1.f);*/

	// Detection Success
	if (Stimulus.WasSuccessfullySensed())
	{
		if (!BB->GetValueAsObject(FName("TargetActor")))
		{
			BB->SetValueAsObject(FName("TargetActor"),Actor);
			BB->SetValueAsBool(FName("bCanAttack"),false);
		}
	}
	else if (BB->GetValueAsObject(FName("TargetActor")) == Actor)
	{
		AActor* Replacement = FindAlternativeTarget(Actor);
		if (Replacement)
		{
			BB->SetValueAsObject(FName("TargetActor"), Replacement);
		}
		else
		{
			BB->ClearValue(FName("TargetActor"));
		}
		BB->SetValueAsBool(FName("bCanAttack"),false);
	}
}

AActor* AGS_EnemyAIController::FindAlternativeTarget(AActor* ExcludeActor) const
{
	APawn* SelfPawn = GetPawn();
	if (!PerceptionComp || !SelfPawn)
	{
		return nullptr;
	}

	TArray<AActor*> PerceivedActors;
	PerceptionComp->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), PerceivedActors);

	AActor* NearestActor = nullptr;
	float NearestDistSq = TNumericLimits<float>::Max();

	for (AActor* Candidate : PerceivedActors)
	{
		if (!Candidate || Candidate == ExcludeActor || UGA_AbilityBase::IsSameTeam(SelfPawn, Candidate))
		{
			continue;
		}

		const UAbilitySystemComponent* CandidateASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Candidate);
		if (CandidateASC && CandidateASC->HasMatchingGameplayTag(StateTag::TAG_State_Dead))
		{
			continue;
		}

		const float DistSq = FVector::DistSquared(SelfPawn->GetActorLocation(), Candidate->GetActorLocation());
		if (DistSq < NearestDistSq)
		{
			NearestDistSq = DistSq;
			NearestActor = Candidate;
		}
	}

	return NearestActor;
}

void AGS_EnemyAIController::DrawDebug_SightRadius()
{
	//TODO::Migrate to EnemyClass Tick
	// if ListenServer Settings Can See DebugLine
	APawn* OwnerPawn = GetPawn();
	if (!OwnerPawn || !Sight_Config)
	{
		return;
	}
	
	AGS_Enemy* Enemy = Cast<AGS_Enemy>(OwnerPawn);
	if (!Enemy)
	{
		return;
	}
	
	// Accept Sense SightRadius
	DrawDebugSphere(GetWorld(),OwnerPawn->GetActorLocation(),Sight_Config->SightRadius,32,FColor::Green,false);
	// Lose Sense SightRadius
	DrawDebugSphere(GetWorld(),OwnerPawn->GetActorLocation(),Sight_Config->LoseSightRadius,32,FColor::Red,false);
	// AttackDistance
	DrawDebugSphere(GetWorld(),OwnerPawn->GetActorLocation(),Enemy->GetEnemyData().AcceptanceRadius,32,FColor::Red,false);
}

