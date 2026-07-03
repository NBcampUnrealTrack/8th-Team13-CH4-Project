// Fill out your copyright notice in the Description page of Project Settings.


#include "GSSpawnManager.h"

#include "GSSpawnPoint.h"
#include "Gang_Squirrel/SpawnSystem/GSPoolSubsystem.h"
#include "Gang_Squirrel/Food/GSFoodBase.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AGSSpawnManager::AGSSpawnManager()
{
	bReplicates = true;
	
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AGSSpawnManager::BeginPlay()
{
	Super::BeginPlay();
	
	SpawnPoints.Empty();
	
	PoolSubsystem = GetWorld()->GetSubsystem<UGSPoolSubsystem>();
	if (!IsValid(PoolSubsystem)) return;
	
	if (HasAuthority())
	{
		PoolSubsystem->InitializePool(DataAssets);
	}
	
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(
		GetWorld(),
		AGSSpawnPoint::StaticClass(),
		FoundActors
	);
	
	for (AActor* Actor : FoundActors)
	{
		if (!IsValid(Actor)) continue;
		SpawnPoints.Add(Cast<AGSSpawnPoint>(Actor));
		
		UE_LOG(LogTemp, Warning, TEXT("Spawned Actor: %s"), *Actor->GetName());
	}
	
	UE_LOG(LogTemp, Warning, TEXT("SPawnPoint Count : %d"), SpawnPoints.Num());
	if (HasAuthority())
	{
		Spawn();
	
		GetWorld()->GetTimerManager().SetTimer(
			SpawnTimer,
			this,
			&AGSSpawnManager::Spawn,
			60.f,
			true,
			60.f
		);
	}
}

void AGSSpawnManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorld()->GetTimerManager().ClearTimer(SpawnTimer);
	
	Super::EndPlay(EndPlayReason);
}

// Called every frame
void AGSSpawnManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AGSSpawnManager::Spawn()
{
	if (!HasAuthority()) return;
	
	int32 RetryCount = 0;
	for (AGSSpawnPoint* SpawnPoint : SpawnPoints)
	{
		if (!IsValid(SpawnPoint))
		{
			UE_LOG(LogTemp, Warning, TEXT("SpawnPoint null"));
			continue;
		}
		for (int i = 0; i < SpawnPoint->MaxSpawnAmount; ++i)
		{
			if (RetryCount > 50)
			{
				break;
			}
			if (SpawnPoint->MaxSpawnAmount > SpawnPoint->CurrentFoodCount)
			{
				FVector CurrentLocation = SpawnPoint->GetRandomLocation();
				if (bCheckArround(CurrentLocation))
				{
					RetryCount++;
					continue;
				}
				
				AGSFoodBase* Food = PoolSubsystem->GetFood();
				if (!IsValid(Food))
				{
					UE_LOG(LogTemp, Warning, TEXT("Food Null"));
					return;
				}
			
				Food->SetActorLocation(CurrentLocation);
				Food->Activate();
			
				UE_LOG(LogTemp, Warning, TEXT("Spawn Actor"));
			
				SpawnPoint->CurrentFoodCount+=1;
			}
			else
			{
				continue;
			}
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("Spawn Work"));
}

bool AGSSpawnManager::bCheckArround(const FVector& CheckLocation) const
{
	TArray<AActor*> OverlappingActors;
	TArray<AActor*> IgnoreActors;
	
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));
	
	bool bOverlap = UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(),
		CheckLocation,
		50.f,
		ObjectTypes,
		AGSFoodBase::StaticClass(),
		IgnoreActors,
		OverlappingActors
		);
	
	if (!bOverlap)
	{
		return false;
	}
	return true;
}

