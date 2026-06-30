#include "GS_EnemyAIController.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AIPerceptionTypes.h"
#include "Perception/AISenseConfig_Sight.h"


AGS_EnemyAIController::AGS_EnemyAIController()
{
	PrimaryActorTick.bCanEverTick = true;
	
	//TODO::DataTable,Asset Refac
	PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
	
#pragma region SightConfig_Settings
	Sight_Config = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	Sight_Config->SightRadius = 1500.f;
	Sight_Config->LoseSightRadius = 2000.f;
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
	
	DrawDebug_SightRadius();
}

void AGS_EnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
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
	
	// Detection Success
	if (Stimulus.WasSuccessfullySensed())
	{
		BB->SetValueAsObject(FName("TargetActor"),Actor);
	}
	// Detection Failed
	else
	{
		BB->ClearValue(FName("TargetActor"));
	}
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
	
	// Accept Sense SightRadius
	DrawDebugSphere(GetWorld(),OwnerPawn->GetActorLocation(),Sight_Config->SightRadius,32,FColor::Green,false);
	// Lose Sense SightRadius
	DrawDebugSphere(GetWorld(),OwnerPawn->GetActorLocation(),Sight_Config->LoseSightRadius,32,FColor::Red,false);
}

