// Fill out your copyright notice in the Description page of Project Settings.


#include "GSSpawnPoint.h"

#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AGSSpawnPoint::AGSSpawnPoint()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	SpawnArea = CreateDefaultSubobject<UBoxComponent>(("SpawnArea"));
	RootComponent = SpawnArea;

}

FVector AGSSpawnPoint::GetRandomLocation() const
{
	const FVector Origin = SpawnArea->GetComponentLocation();
	const FVector Extent = SpawnArea->GetScaledBoxExtent();
	
	FVector ResultLocation = UKismetMathLibrary::RandomPointInBoundingBox(Origin, Extent);
	FVector Start = ResultLocation + FVector(0.f,0.f,1.f);
	FVector End = ResultLocation - FVector(0.f, 0.f, 100.f);
	
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
