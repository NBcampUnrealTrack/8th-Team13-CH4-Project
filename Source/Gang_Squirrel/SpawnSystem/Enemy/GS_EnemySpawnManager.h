#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gang_Squirrel/Enemy/GS_Enemy.h"
#include "GS_EnemySpawnManager.generated.h"

class ANavMeshBoundsVolume;
class UBoxComponent;

UCLASS()
class GANG_SQUIRREL_API AGS_EnemySpawnManager : public AActor
{
	GENERATED_BODY()
public:
	AGS_EnemySpawnManager();
	
	virtual void BeginPlay() override;
	
	UFUNCTION(BlueprintCallable)
	AGS_Enemy* SpawnEnemy(const FVector& Location, const FRotator& Rotation);
	UFUNCTION(BlueprintCallable)
	void ReturnEnemyToPool(AGS_Enemy* Enemy);
	UFUNCTION(BlueprintCallable)
	AGS_Enemy* SpawnEnemyAtRandomLocation();
	
	UFUNCTION(BlueprintCallable)
	void StartSpawnEnemy();
	UFUNCTION(BlueprintCallable)
	void StopSpawnEnemy();
	
protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> SpawnArea;
	
	UPROPERTY(EditDefaultsOnly,Category="Pool")
	TSubclassOf<AGS_Enemy> EnemyClass;
	UPROPERTY(EditDefaultsOnly,Category="Pool")
	int32 PoolSize = 5;
	
	UPROPERTY(EditDefaultsOnly,Category="Spawn")
	int32 MaxRandomLocationAttempts = 10;
	UPROPERTY(EditDefaultsOnly,Category="Spawn")
	FVector NavQueryExtent = FVector(50.f,50.f,100.f);
	UPROPERTY(EditDefaultsOnly,Category="Spawn")
	float SpawnInterval = 5.f;
	UPROPERTY(EditDefaultsOnly,Category="Spawn")
	float FirstSpawnDelay = 1.f;
	
	UPROPERTY()
	TArray<TObjectPtr<AGS_Enemy>> Pool;
	UPROPERTY()
	TArray<TObjectPtr<ANavMeshBoundsVolume>> OverlappingNavVolumes;
	
private:
	void PrewarmPool();
	void FindOverlappingNavVolumes();
	
	bool TryGetRandomNavigablePoint(FVector& OutLocation) const;
	
	FTimerHandle SpawnTimerHandle;
	void SpawnEnemyTick();
};
