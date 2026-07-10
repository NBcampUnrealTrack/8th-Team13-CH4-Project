// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GS_GameModeBase.generated.h"

class AGSSpawnManager;
class AGS_PlayerState;
class UGSFoodPrimaryDataAsset;   
class AGSCharacter;
class UGameplayEffect;
/**
 * 
 */

UENUM(BlueprintType)
enum class ERewardType : uint8
{
	Food,
	Capacity,
	SpeedBoost
};

UCLASS()
class GANG_SQUIRREL_API AGS_GameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:

	AGS_GameModeBase();

protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable, Category = "Match")
	void StartMatch();

	UFUNCTION(BlueprintCallable, Category = "Match")
	void EndMatch();

	//Notify completed Player nickname
	void NotifyPlayerReady();

	UFUNCTION(BlueprintCallable, Category = "Reward")
	void GiveRandomReward(AGS_PlayerState* KillerPS);

	// 테스트 디버그용 - 타입 직접 지정
	UFUNCTION(BlueprintCallable, Category = "Reward")
	void GiveSpecificReward(AGS_PlayerState* KillerPS, ERewardType RewardType);

protected:
	//Call when timer == 0.f
	void OnMatchTimeExpired();

	//Multicast 
	UFUNCTION(NetMulticast, Unreliable)
	void MultiCastRPCPrintStatus(int32 Ready, int32 Total);

protected:

	UPROPERTY(EditDefaultsOnly, Category = "Match")
	float MatchTimeLimit;

	//boolean property
	UPROPERTY(BlueprintReadOnly, Category = "Match")
	uint8 bMatchEnd : 1;

private:
	//Match Timer
	FTimerHandle MatchTimerHandle;

	int32 ReadyPlayerCount = 0;
public:
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AGSSpawnManager> SpawnManagerClass;
	
	void SpawnSpawnManager() const;

private:
	void GiveFoodReward(AGS_PlayerState* PS);

	void GiveCapacityReward(AGS_PlayerState* PS);

	void GiveSpeedBoostReward(AGS_PlayerState* PS);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Reward|Food")
	TObjectPtr<UGSFoodPrimaryDataAsset> RewardFoodData;

	UPROPERTY(EditDefaultsOnly, Category = "Reward|Capacity")
	float CapacityIncreaseAmount = 0.1f;


	UPROPERTY(EditDefaultsOnly, Category = "Reward|SpeedBoost")
	TSubclassOf<UGameplayEffect> GE_MoveSpeedRewardClass;
};
