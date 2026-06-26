// Fill out your copyright notice in the Description page of Project Settings.


#include "GSFoodBase.h"

#include "NaniteSceneProxy.h"
#include "Gang_Squirrel/DataAsset/GSFoodPrimaryDataAsset.h"
#include "Components/SphereComponent.h"

// Sets default values
AGSFoodBase::AGSFoodBase()
{
	
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("StaticMeshComponent");
	RootComponent = StaticMeshComponent;
	SphereComponent = CreateDefaultSubobject<USphereComponent>("OverlapCollision");
	SphereComponent->SetupAttachment(StaticMeshComponent);
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	

}

// Called when the game starts or when spawned
void AGSFoodBase::BeginPlay()
{
	Super::BeginPlay();
	
	Init(FoodData);
}

// Called every frame
void AGSFoodBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AGSFoodBase::Init(UGSFoodPrimaryDataAsset* InData)
{
	FoodData = InData;
	
	StaticMeshComponent->SetStaticMesh(FoodData->FoodMesh);
}

