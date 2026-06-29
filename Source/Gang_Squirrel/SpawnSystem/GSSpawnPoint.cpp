// Fill out your copyright notice in the Description page of Project Settings.


#include "GSSpawnPoint.h"

#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AGSSpawnPoint::AGSSpawnPoint()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	SpawnArea = CreateDefaultSubobject<UBoxComponent>(("SpawnArea"));
	RootComponent = SpawnArea;

}

// Called when the game starts or when spawned
void AGSSpawnPoint::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGSSpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

FVector AGSSpawnPoint::GetRandomLocation() const
{
	const FVector Origin = SpawnArea->GetComponentLocation();
	const FVector Extent = SpawnArea->GetScaledBoxExtent();
	
	FVector ResultLocation = UKismetMathLibrary::RandomPointInBoundingBox(Origin, Extent);
	FVector Start = ResultLocation + FVector(0.f,0.f,1000.f);
	FVector End = ResultLocation - FVector(0.f, 0.f, 5000.f);
	
	FHitResult Hit;
	
	bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit,
		Start,
		End,
		ECC_Visibility
	);
	
	if (bHit)
	{
		ResultLocation.Z = Hit.ImpactPoint.Z;
	}
	
	return ResultLocation;
}
