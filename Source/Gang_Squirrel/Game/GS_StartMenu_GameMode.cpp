// Fill out your copyright notice in the Description page of Project Settings.


#include "Gang_Squirrel/Game/GS_StartMenu_GameMode.h"


void AGS_StartMenu_GameMode::OnLobbyPlayerReady()
{
    LobbyReadyCount++;
    UE_LOG(LogTemp, Warning, TEXT("[Lobby] Player ready : %d / %d"), LobbyReadyCount, GetNumPlayers());

    if (LobbyReadyCount >= GetNumPlayers())
    {
        GetWorld()->ServerTravel("/Game/ProjectFile/Level/L_Main_Stage");
    }
}
