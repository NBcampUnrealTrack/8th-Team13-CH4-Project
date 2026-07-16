// Fill out your copyright notice in the Description page of Project Settings.


#include "GSPoolSubsystem.h"

#include "Gang_Squirrel/Food/GSFoodBase.h"
#include "Gang_Squirrel/DataAsset/GSFoodPrimaryDataAsset.h"
#include "Gang_Squirrel/SpawnSystem/GSSpawnManager.h"
#include "Algo/RandomShuffle.h"

void UGSPoolSubsystem::InitializePool(const TArray<FFoodSpawnInfo>& DataAssets)
{
	FoodPool.Empty();
	
	int32 TotalCount = 0;
	
	for (const FFoodSpawnInfo&  SpawnInfo : DataAssets)
	{
		TotalCount += SpawnInfo.SpawnAmount;
	}
	
	FoodPool.Reserve(TotalCount);
	
	for (const FFoodSpawnInfo& SpawnInfo : DataAssets)
	{
		UGSFoodPrimaryDataAsset* CurrentDataAsset = SpawnInfo.FoodData;
		int32 CurrentSpawnAmount = SpawnInfo.SpawnAmount;
		if (!CurrentDataAsset || !CurrentSpawnAmount) continue;
		
		for (int32 j = 0; j < CurrentSpawnAmount; ++j)
		{
			AGSFoodBase* Food = GetWorld()->SpawnActor<AGSFoodBase>();
			if (!IsValid(Food)) continue;
			
			Food->Init(CurrentDataAsset);
			Food->Deactivate();
			FoodPool.Add(Food);
			// UE_LOG(LogTemp, Warning, TEXT("Spawn Food!!Actor"));
		}
		
	}
	
	Algo::RandomShuffle(FoodPool);
}

AGSFoodBase* UGSPoolSubsystem::GetFood(int32 TargetFloor)
{
	if (FoodPool.IsEmpty()) return nullptr;
	
	for (AGSFoodBase* Food : FoodPool)
	{
		if (!Food->bIsActive && IsValid(Food) && Food->FoodData->CurrentFloor == TargetFloor) return Food;
	}
	
	return nullptr;
}
