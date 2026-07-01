#include "GS_FallingHazardManager.h"

#include "GS_FallingHazard.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AGS_FallingHazardManager::AGS_FallingHazardManager()
{
	PrimaryActorTick.bCanEverTick = false;

	/*
	 * 매니저 자체도 서버/클라에 존재할 수 있게 복제 가능하게 둔다.
	 * 다만 실제 Spawn 로직은 서버에서만 실행한다.
	 */
	bReplicates = true;
}

void AGS_FallingHazardManager::BeginPlay()
{
	Super::BeginPlay();

	/*
	 * 중요:
	 * 낙하물 Spawn은 반드시 서버에서만 해야 한다.
	 * 클라이언트에서도 Spawn하면 서버가 모르는 가짜 낙하물이 생긴다.
	 */
	if (HasAuthority() == false)
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

	const FVector SpawnLocation = TargetActor->GetActorLocation() + FVector(0.f, 0.f, 1000.f);
	const FRotator SpawnRotation = FRotator::ZeroRotator;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;

	/*
	 * 서버에서 Spawn한 replicated actor는 클라이언트에도 생성된다.
	 */
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

	/*
	 * 현재는 ACharacter를 전부 후보로 가져온다.
	 * 만약 나중에 AI 다람쥐나 NPC도 ACharacter라면,
	 * 여기서 AGSCharacter만 찾도록 바꾸는 게 좋다.
	 */
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