// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GS_StartMenu_GameMode.generated.h"

/**
 * 
 */
UCLASS()
class GANG_SQUIRREL_API AGS_StartMenu_GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AGS_StartMenu_GameMode();

	void OnLobbyPlayerReady();

private:
	int32 LobbyReadyCount = 0;

};
