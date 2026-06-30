#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "GS_EnemyAIController.generated.h"

class UAISenseConfig_Sight;
struct FAIStimulus;
//TODO::AIPerceptionComp,SightConfig,BTRun
UCLASS()
class GANG_SQUIRREL_API AGS_EnemyAIController : public AAIController
{
	GENERATED_BODY()
public:
	AGS_EnemyAIController();
	
protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaSeconds) override;
	
	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);
	
	UPROPERTY(EditDefaultsOnly,Category="AI|BT")
	TObjectPtr<UBehaviorTree> BT_Enemy;
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAIPerceptionComponent> PerceptionComp;
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAISenseConfig_Sight> Sight_Config;
private:
	//Debugging Sight Radius Func
	void DrawDebug_SightRadius();
};
