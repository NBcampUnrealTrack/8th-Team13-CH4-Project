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

	void StartSpawnFallingHazard();
	void StopSpawnFallingHazard();

protected:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Falling Hazard")
	TSubclassOf<AGS_FallingHazard> FallingHazardClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Falling Hazard")
	float SpawnInterval = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Falling Hazard")
	float FirstSpawnDelay = 1.f;

	FTimerHandle SpawnTimerHandle;

protected:
	void SpawnFallingHazard();

	AActor* FindTargetActor() const;
};