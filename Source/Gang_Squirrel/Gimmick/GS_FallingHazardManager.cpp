#include "GS_FallingHazardManager.h"

#include "GS_FallingHazard.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "GameFramework/Character.h"

AGS_FallingHazardManager::AGS_FallingHazardManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AGS_FallingHazardManager::BeginPlay()
{
	Super::BeginPlay();

	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(this, 0);
	TargetActor = Cast<AActor>(PlayerCharacter);

	GetWorldTimerManager().SetTimer(
		SpawnTimerHandle,
		this,
		&AGS_FallingHazardManager::SpawnFallingHazard,
		SpawnInterval,
		true
	);
}

void AGS_FallingHazardManager::SpawnFallingHazard()
{
	if (IsValid(TargetActor) == false)
	{
		return;
	}

	if (FallingHazardClass == nullptr)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (IsValid(World) == false)
	{
		return;
	}

	const FVector SpawnLocation = TargetActor->GetActorLocation() + FVector(0.f, 0.f, 1000.f);
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