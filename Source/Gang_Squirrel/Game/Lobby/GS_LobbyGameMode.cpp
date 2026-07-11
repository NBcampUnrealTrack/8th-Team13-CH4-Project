#include "GS_LobbyGameMode.h"

#include "Gang_Squirrel/EOS/GS_GameInstance.h"
#include "Gang_Squirrel/Player/GS_PlayerState.h"

void AGS_LobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
	if (AGS_PlayerState* PS = NewPlayer ? NewPlayer->GetPlayerState<AGS_PlayerState>() : nullptr)
	{
		PS->bIsHost = NewPlayer->IsLocalController();
	}
}

void AGS_LobbyGameMode::TryStartGame(APlayerController* Requester)
{
	if (!HasAuthority() || !Requester)
	{
		return;
	}
	
	const AGS_PlayerState* RequesterPS = Requester->GetPlayerState<AGS_PlayerState>();
	if (!RequesterPS || !RequesterPS->bIsHost)
	{
		return;
	}
	
	if (!CanStartGame())
	{
		return;
	}
	
	if (UGS_GameInstance* GSInstance = Cast<UGS_GameInstance>(GetGameInstance()))
	{
		GSInstance->StartGame(MainStageLevelName);
	}
}

bool AGS_LobbyGameMode::CanStartGame()
{
	return GetNumPlayers() >= MinPlayersToStart;
}
