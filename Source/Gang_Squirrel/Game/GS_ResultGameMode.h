// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GS_ResultGameMode.generated.h"

/**
 * 
 */
UCLASS()
class GANG_SQUIRREL_API AGS_ResultGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AGS_ResultGameMode();

	//void OnLobbyPlayerReady();

protected:

	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

private:
	//int32 LobbyReadyCount = 0;

};
