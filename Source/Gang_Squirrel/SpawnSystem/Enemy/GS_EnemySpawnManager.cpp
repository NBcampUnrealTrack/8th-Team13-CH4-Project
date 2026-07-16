#include "GS_EnemySpawnManager.h"

#include "NavigationSystem.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "NavMesh/NavMeshBoundsVolume.h"


AGS_EnemySpawnManager::AGS_EnemySpawnManager()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	
	SpawnArea = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnArea"));
	RootComponent = SpawnArea;
	SpawnArea->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AGS_EnemySpawnManager::BeginPlay()
{
	Super::BeginPlay();
	
	if (!HasAuthority())
	{
		return;
	}
	
	FindOverlappingNavVolumes();
	PrewarmPool();
}

AGS_Enemy* AGS_EnemySpawnManager::SpawnEnemy(const FVector& Location, const FRotator& Rotation)
{
	if (!HasAuthority())
	{
		return nullptr;
	}
	
	for (AGS_Enemy* Enemy : Pool)
	{
		if (Enemy && !Enemy->IsPoolActive())
		{
			Enemy->ActivateEnemy(Location,Rotation);
			return Enemy;
		}
	}
	
	if (AGS_Enemy* Extra = GetWorld()->SpawnActor<AGS_Enemy>(EnemyClass,Location,Rotation))
	{
		Extra->SetOwningSpawnManager(this);
		Pool.Add(Extra);
		Extra->ActivateEnemy(Location,Rotation);
		return Extra;
	}
	
	return nullptr;
}

void AGS_EnemySpawnManager::ReturnEnemyToPool(AGS_Enemy* Enemy)
{
	if (Enemy)
	{
		Enemy->DeactivateEnemy();
	}
}

void AGS_EnemySpawnManager::PrewarmPool()
{
	if (!EnemyClass)
	{
		return;
	}
	
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	for (int32 i = 0; i < PoolSize; ++i)
	{
		if (AGS_Enemy* Enemy = GetWorld()->SpawnActor<AGS_Enemy>(EnemyClass,FVector::ZeroVector,FRotator::ZeroRotator,Params))
		{
			Enemy->SetOwningSpawnManager(this);
			Enemy->DeactivateEnemy();
			Pool.Add(Enemy);
		}
	}
}

void AGS_EnemySpawnManager::FindOverlappingNavVolumes()
{
	OverlappingNavVolumes.Reset();
	
	const FBox MyBounds = SpawnArea->Bounds.GetBox();
	
	TArray<AActor*> AllVolumes;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ANavMeshBoundsVolume::StaticClass(), AllVolumes);
	
	for (AActor* Actor: AllVolumes)
	{
		ANavMeshBoundsVolume* Volume = Cast<ANavMeshBoundsVolume>(Actor);
		if (!Volume)
		{
			continue;
		}
		
		const FBox VolumeBounds = Volume->GetComponentsBoundingBox(true);
		if (MyBounds.Intersect(VolumeBounds))
		{
			OverlappingNavVolumes.Add(Volume);
		}
	}
}

AGS_Enemy* AGS_EnemySpawnManager::SpawnEnemyAtRandomLocation()
{
	if (!HasAuthority())
	{
		return nullptr;
	}
	
	FVector RandomLocation;
	if (!TryGetRandomNavigablePoint(RandomLocation))
	{
		return nullptr;
	}
	
	return SpawnEnemy(RandomLocation, FRotator::ZeroRotator);
}

bool AGS_EnemySpawnManager::TryGetRandomNavigablePoint(FVector& OutLocation) const
{
	if (OverlappingNavVolumes.Num() == 0)
	{
		return false;
	}
	
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!NavSys)
	{
		return false;
	}
	
	for (int32 Attempt = 0; Attempt < MaxRandomLocationAttempts; ++Attempt)
	{
		const ANavMeshBoundsVolume* Volume = OverlappingNavVolumes[FMath::RandRange(0, OverlappingNavVolumes.Num() - 1)];
		const FBox VolumeBounds = Volume->GetComponentsBoundingBox(true);
		
		const FVector RandomPoint = UKismetMathLibrary::RandomPointInBoundingBox(VolumeBounds.GetCenter(), VolumeBounds.GetExtent());
		
		FNavLocation NavLocation;
		if (NavSys->ProjectPointToNavigation(RandomPoint, NavLocation,NavQueryExtent))
		{
			OutLocation = NavLocation.Location;
			return true;
		}
	}
	
	return false;
}

void AGS_EnemySpawnManager::StartSpawnEnemy()
{
	if (!HasAuthority())
	{
		return;
	}
	
	if (GetWorldTimerManager().IsTimerActive(SpawnTimerHandle))
	{
		return;
	}
	
	GetWorldTimerManager().SetTimer(SpawnTimerHandle,this,&AGS_EnemySpawnManager::SpawnEnemyTick,SpawnInterval,true,FirstSpawnDelay);
}

void AGS_EnemySpawnManager::StopSpawnEnemy()
{
	if (!HasAuthority())
	{
		return;
	}
	
	GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
}

void AGS_EnemySpawnManager::SpawnEnemyTick()
{
	if (!HasAuthority())
	{
		return;
	}
	
	const bool bHasInactiveSlot = Pool.ContainsByPredicate([](const AGS_Enemy* Enemy)
	{
		return Enemy && !Enemy->IsPoolActive();
	});
	
	if (!bHasInactiveSlot)
	{
		return;
	}
	
	SpawnEnemyAtRandomLocation();
}