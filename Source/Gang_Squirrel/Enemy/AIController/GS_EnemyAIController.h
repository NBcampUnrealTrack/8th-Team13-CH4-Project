#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "GS_EnemyAIController.generated.h"

UCLASS()
class GANG_SQUIRREL_API AGS_EnemyAIController : public AAIController
{
	GENERATED_BODY()
public:
	AGS_EnemyAIController();
protected:
	virtual void BeginPlay() override;
public:
	virtual void Tick(float DeltaTime) override;
};
