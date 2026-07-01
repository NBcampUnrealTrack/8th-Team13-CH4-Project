// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "GS_GameState.generated.h"

/**
 * 
 */
UCLASS()
class GANG_SQUIRREL_API AGS_GameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UFUNCTION()
	void OnRep_MatchEndTime();

public:

	//Match end time at server.
	UPROPERTY(ReplicatedUsing = OnRep_MatchEndTime, BlueprintReadOnly, Category = "Match")
	float MatchEndTime;

	UFUNCTION(BlueprintPure, Category = "Match")
	float GetRemainingTime() const;

	UFUNCTION(BlueprintPure, Category = "Match")
	FString GetRemainingTimeAsString() const;

};
