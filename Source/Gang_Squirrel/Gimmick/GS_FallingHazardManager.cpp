#include "GS_FallingHazardManager.h"

#include "GS_FallingHazard.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AGS_FallingHazardManager::AGS_FallingHazardManager()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
}

void AGS_FallingHazardManager::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() == false)
	{
		return;
	}
}

void AGS_FallingHazardManager::StartSpawnFallingHazard()
{
	if (HasAuthority() == false)
	{
		return;
	}

	if (GetWorldTimerManager().IsTimerActive(SpawnTimerHandle))
	{
		return;
	}

	GetWorldTimerManager().SetTimer(
		SpawnTimerHandle,
		this,
		&AGS_FallingHazardManager::SpawnFallingHazard,
		SpawnInterval,
		true,
		FirstSpawnDelay
	);
}

void AGS_FallingHazardManager::StopSpawnFallingHazard()
{
	if (HasAuthority() == false)
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
}

void AGS_FallingHazardManager::SpawnFallingHazard()
{
	if (HasAuthority() == false)
	{
		return;
	}

	if (FallingHazardClass == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("FallingHazardClass is nullptr."));
		return;
	}

	AActor* TargetActor = FindTargetActor();
	if (IsValid(TargetActor) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("FallingHazard target actor is invalid."));
		return;
	}

	UWorld* World = GetWorld();
	if (IsValid(World) == false)
	{
		return;
	}

	const FVector SpawnLocation = TargetActor->GetActorLocation() + FVector(0.f, 0.f, 50.f);
	const FRotator SpawnRotation = FRotator::ZeroRotator;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;

	AGS_FallingHazard* FallingHazard = World->SpawnActor<AGS_FallingHazard>(
		FallingHazardClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParams
	);

	if (IsValid(FallingHazard) == true)
	{
		FallingHazard->SetTargetActor(TargetActor);
	}
}

AActor* AGS_FallingHazardManager::FindTargetActor() const
{
	UWorld* World = GetWorld();
	if (IsValid(World) == false)
	{
		return nullptr;
	}

	TArray<AActor*> PlayerCharacters;

	UGameplayStatics::GetAllActorsOfClass(
		World,
		ACharacter::StaticClass(),
		PlayerCharacters
	);

	if (PlayerCharacters.Num() <= 0)
	{
		return nullptr;
	}

	const int32 RandomIndex = FMath::RandRange(0, PlayerCharacters.Num() - 1);
	return PlayerCharacters[RandomIndex];
}