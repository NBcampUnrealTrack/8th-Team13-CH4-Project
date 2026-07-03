// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GSPoolSubsystem.generated.h"

struct FFoodSpawnInfo;
class UGSFoodPrimaryDataAsset;
class AGSFoodBase;
/**
 * 
 */
UCLASS()
class GANG_SQUIRREL_API UGSPoolSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
	UPROPERTY()
	TArray<TObjectPtr<AGSFoodBase>> FoodPool;
	
public:
	
	void InitializePool(TArray<FFoodSpawnInfo> DataAssets);
	
	AGSFoodBase* GetFood() const;
	
};
