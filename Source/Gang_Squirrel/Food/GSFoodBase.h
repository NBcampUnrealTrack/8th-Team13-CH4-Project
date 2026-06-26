// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GSFoodBase.generated.h"

class UGSFoodPrimaryDataAsset;
class USphereComponent;

UCLASS()
class GANG_SQUIRREL_API AGSFoodBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGSFoodBase();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UGSFoodPrimaryDataAsset> FoodData;
	
	void Init(UGSFoodPrimaryDataAsset* InData);
	
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USphereComponent> SphereComponent;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
