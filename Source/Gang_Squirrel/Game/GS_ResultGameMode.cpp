// Fill out your copyright notice in the Description page of Project Settings.


#include "Gang_Squirrel/Game/GS_ResultGameMode.h"
#include "Gang_Squirrel/Controller/GSPlayerController.h" 

AGS_ResultGameMode::AGS_ResultGameMode()
{
	bUseSeamlessTravel = true;
}

//void AGS_StartMenu_GameMode::OnLobbyPlayerReady()
//{
//    LobbyReadyCount++;
//    UE_LOG(LogTemp, Warning, TEXT("[Lobby] Player ready : %d / %d"), LobbyReadyCount, GetNumPlayers());
//
//    if (LobbyReadyCount >= GetNumPlayers())
//    {
//        GetWorld()->ServerTravel("/Game/ProjectFile/Level/L_Main_Stage");
//    }
//}

void AGS_ResultGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
    Super::HandleStartingNewPlayer_Implementation(NewPlayer);

    if (AGSPlayerController* GSPC = Cast<AGSPlayerController>(NewPlayer))
    {
        GSPC->ClientShowResultStage();
    }
}
