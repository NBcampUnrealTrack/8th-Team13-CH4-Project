// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GSFoodPrimaryDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class GANG_SQUIRREL_API UGSFoodPrimaryDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName FoodName;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 ScoreAmount;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMesh> FoodMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float EatTime = 0.1f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float SquirrelScale = 0.1f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MeshSize = 0.5f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MeshZ = 0.1f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 CurrentFloor = 1;
	
};
