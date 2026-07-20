// Fill out your copyright notice in the Description page of Project Settings.


#include "GS_GameState.h"
#include "Net/UnrealNetwork.h"

void AGS_GameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGS_GameState, MatchEndTime);
}

void AGS_GameState::OnRep_MatchEndTime()
{
	//Copy MatchEndTime at client Complited
	UE_LOG(LogTemp, Log, TEXT("[Client] MatchEndTime Received: %.1f"), MatchEndTime);
	
	//Add Other Code
}

float AGS_GameState::GetRemainingTime() const
{
	float Remaining = MatchEndTime - GetServerWorldTimeSeconds();

	return FMath::Max(Remaining, 0.0f);
}

FString AGS_GameState::GetRemainingTimeAsString() const
{
	float RemainingTime = GetRemainingTime();

	int32 Minutes = FMath::FloorToInt(RemainingTime / 60.0f);
	int32 Seconds = FMath::FloorToInt(RemainingTime) % 60;

	return FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
}
