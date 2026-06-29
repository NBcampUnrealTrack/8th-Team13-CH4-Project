#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GS_FallingHazardManager.generated.h"

class AGS_FallingHazard;

UCLASS()
class GANG_SQUIRREL_API AGS_FallingHazardManager : public AActor
{
	GENERATED_BODY()

public:
	AGS_FallingHazardManager();

	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Falling Hazard")
	TSubclassOf<AGS_FallingHazard> FallingHazardClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Falling Hazard")
	float SpawnInterval = 5.f;

	UPROPERTY()
	TObjectPtr<AActor> TargetActor;

	FTimerHandle SpawnTimerHandle;

protected:
	void SpawnFallingHazard();
};