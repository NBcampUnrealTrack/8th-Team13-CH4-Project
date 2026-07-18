#include "GS_LobbyGameMode.h"

#include "Gang_Squirrel/EOS/GS_GameInstance.h"
#include "Gang_Squirrel/Game/GS_GameState.h"
#include "Gang_Squirrel/Player/GS_PlayerState.h"
#include "Gang_Squirrel/Controller/Lobby/GS_LobbyPlayerController.h"

AGS_LobbyGameMode::AGS_LobbyGameMode()
{
	GameStateClass = AGS_GameState::StaticClass();
	DefaultPawnClass = nullptr;
	bUseSeamlessTravel = true;
}

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
	
	if (GetWorldTimerManager().IsTimerActive(StartGameTimerHandle))
	{
		return;
	}

	GetWorldTimerManager().SetTimer(
		StartGameTimerHandle,
		this,
		&AGS_LobbyGameMode::StartTravelToMainStage,
		0.3f,
		false
	);

	for (
		FConstPlayerControllerIterator It =
		GetWorld()->GetPlayerControllerIterator();
		It;
		++It
		)
	{
		AGS_LobbyPlayerController* LobbyPC =
			Cast<AGS_LobbyPlayerController>(It->Get());

		if (!LobbyPC)
		{
			continue;
		}

		LobbyPC->ClientStartLoadingScreen();
	}


}

bool AGS_LobbyGameMode::CanStartGame()
{
	if (GetNumPlayers() < MinPlayersToStart)
	{
		return false;
	}
	
	const AGS_GameState* GS = GetGameState<AGS_GameState>();
	if (!GS)
	{
		return false;
	}
	
	for (APlayerState* PS : GS->PlayerArray)
	{
		const AGS_PlayerState* CandidatePS = Cast<AGS_PlayerState>(PS);
		if (!CandidatePS || CandidatePS->bIsHost)
		{
			continue;
		}
		if (!CandidatePS->bIsReady)
		{
			return false;
		}
	}
	return true;
}

void AGS_LobbyGameMode::StartTravelToMainStage()
{
	if (!HasAuthority())
	{
		return;
	}

	UGS_GameInstance* GSInstance =
		Cast<UGS_GameInstance>(GetGameInstance());

	if (!GSInstance)
	{
	
		return;
	}

	GSInstance->StartGame(MainStageLevelName);
}