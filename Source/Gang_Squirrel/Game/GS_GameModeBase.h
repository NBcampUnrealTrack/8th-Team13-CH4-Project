// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GS_GameModeBase.generated.h"

/**
 * 
 */
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
};
