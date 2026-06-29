// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GSSpawnManager.generated.h"

class UGSFoodPrimaryDataAsset;
class UGSPoolSubsystem;
class AGSSpawnPoint;


USTRUCT(BlueprintType)
struct FFoodSpawnInfo
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UGSFoodPrimaryDataAsset> FoodData = nullptr;
	
	UPROPERTY(EditAnywhere)
	int32 SpawnAmount = 0;
};

UCLASS()
class GANG_SQUIRREL_API AGSSpawnManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGSSpawnManager();
	
	UPROPERTY()
	TArray<TObjectPtr<AGSSpawnPoint>> SpawnPoints;
	
	UPROPERTY(EditAnywhere)
	TArray<FFoodSpawnInfo> DataAssets;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	void Spawn();
	
private:
	
	UPROPERTY()
	UGSPoolSubsystem* PoolSubsystem;

};
