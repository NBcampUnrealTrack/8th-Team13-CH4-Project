// Fill out your copyright notice in the Description page of Project Settings.


#include "GS_GameModeBase.h"
#include "Gang_Squirrel/Game/GS_GameState.h"

AGS_GameModeBase::AGS_GameModeBase()
{
	MatchTimeLimit = 600.f;
}

void AGS_GameModeBase::BeginPlay()
{
	Super::BeginPlay();

	StartMatch();
}

void AGS_GameModeBase::StartMatch()
{
	bMatchEnd = false;

	AGS_GameState* GS = GetGameState<AGS_GameState>();
	if (GS)
	{
		GS->MatchEndTime = GetWorld()->GetTimeSeconds() + MatchTimeLimit;
	}
	
		GetWorldTimerManager().SetTimer(
			MatchTimerHandle,
			this,
			&AGS_GameModeBase::OnMatchTimeExpired,
			MatchTimeLimit,
			false
		);
	
	UE_LOG(LogTemp, Log, TEXT("[Server] Match Started. %.1f sec"), MatchTimeLimit)
}

void AGS_GameModeBase::EndMatch()
{
	if (bMatchEnd)
	{
		return;
	}

	bMatchEnd = true;
	GetWorldTimerManager().ClearTimer(MatchTimerHandle);

	UE_LOG(LogTemp, Log, TEXT("[Server] Match Ended."))
}

void AGS_GameModeBase::OnMatchTimeExpired()
{
	EndMatch();
}
