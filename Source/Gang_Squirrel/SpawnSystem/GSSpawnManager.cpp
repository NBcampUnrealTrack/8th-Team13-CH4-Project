// Fill out your copyright notice in the Description page of Project Settings.


#include "GSSpawnManager.h"

#include "GSSpawnPoint.h"
#include "Gang_Squirrel/DataAsset/GSFoodPrimaryDataAsset.h"
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
		
		// UE_LOG(LogTemp, Warning, TEXT("Spawned Actor: %s"), *Actor->GetName());
	}
	
	// UE_LOG(LogTemp, Warning, TEXT("SPawnPoint Count : %d"), SpawnPoints.Num());
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


void AGSSpawnManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AGSSpawnManager::Spawn()
{
    if (!HasAuthority()) return;
    if (!IsValid(PoolSubsystem)) return;
    
    for (AGSSpawnPoint* SpawnPoint : SpawnPoints)
    {
       if (!IsValid(SpawnPoint))
       {
          UE_LOG(LogTemp, Warning, TEXT("SpawnPoint null"));
          continue;
       }

       int32 DesiredSpawnAmount = SpawnPoint->MaxSpawnAmount - SpawnPoint->CurrentFoodCount;
       if (DesiredSpawnAmount <= 0) continue;

       for (int32 SpawnIdx = 0; SpawnIdx < DesiredSpawnAmount; ++SpawnIdx)
       {
          FVector SpawnLocation = FVector::ZeroVector;
          bool bFoundLocation = false;
          int32 LocationRetryCount = 0;
          
          while (LocationRetryCount < 10)
          {
             FVector TargetLoc = SpawnPoint->GetRandomLocation();
        
             if (!bCheckArround(TargetLoc))
             {
                SpawnLocation = TargetLoc;
                bFoundLocation = true;
                break;
             }
             LocationRetryCount++;
          }

          if (!bFoundLocation)
          {
             UE_LOG(LogTemp, Log, TEXT("SpawnPoint [%s] failed to find clear location"), *SpawnPoint->GetName());
             continue; 
          }
       	
          AGSFoodBase* Food = PoolSubsystem->GetFood(SpawnPoint->Floor);
       	
          if (!IsValid(Food)) 
          {
             UE_LOG(LogTemp, Warning, TEXT("Pool Subsystem returned null food for Floor %d"), SpawnPoint->Floor);
             break;
          }
       	
          if (!IsValid(Food->FoodData))
          {
             UE_LOG(LogTemp, Error, TEXT("Food 오브젝트의 FoodData가 nullptr입니다!"));
             continue; 
          }
       	
          UGSFoodPrimaryDataAsset* CurrentFoodData = Food->FoodData;
          SpawnLocation.Z += CurrentFoodData->MeshZ;
          
          Food->SetActorLocation(SpawnLocation);
          Food->Activate();
          
          SpawnPoint->CurrentFoodCount += 1;
       }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Spawn Work Completed"));
}

bool AGSSpawnManager::bCheckArround(const FVector& CheckLocation) const
{
	TArray<AActor*> OverlappingActors;
	TArray<AActor*> IgnoreActors;
	
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));
	
	bool bOverlap = UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(),
		CheckLocation,
		30.f,
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

