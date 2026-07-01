#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GS_FallingHazardManager.generated.h"

class AGS_FallingHazard;

UCLASS()
class GANG_SQUIRREL_API AGS_FallingHazardManager : public AActor
{
	GENERATED_BODY()

public:
	AGS_FallingHazardManager();

	virtual void BeginPlay() override;

protected:
	/*
	 * 스폰할 낙하물 클래스.
	 * BP_FallingHazard 같은 블루프린트 자식을 여기에 넣으면 된다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Falling Hazard")
	TSubclassOf<AGS_FallingHazard> FallingHazardClass;

	/*
	 * 몇 초마다 낙하물을 생성할지.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Falling Hazard")
	float SpawnInterval = 5.f;

	/*
	 * 첫 스폰을 몇 초 뒤에 시작할지.
	 * 0이면 BeginPlay 직후 바로 타이머가 시작된다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Falling Hazard")
	float FirstSpawnDelay = 1.f;

	FTimerHandle SpawnTimerHandle;

protected:
	void SpawnFallingHazard();

	/*
	 * 현재 게임에 존재하는 플레이어 중 타겟 하나를 고르는 함수.
	 * 지금은 랜덤 플레이어를 고르게 했다.
	 */
	AActor* FindTargetActor() const;
};