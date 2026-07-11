#include "GS_LobbyPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "Gang_Squirrel/EOS/GS_GameInstance.h"
#include "Gang_Squirrel/Game/Lobby/GS_LobbyGameMode.h"

void AGS_LobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	if (!IsLocalController())
	{
		return;
	}
	
	if (LobbyWidgetClass)
	{
		UUserWidget* LobbyWidget = CreateWidget<UUserWidget>(this, LobbyWidgetClass);
		if (LobbyWidget)
		{
			LobbyWidget->AddToViewport();
			SetShowMouseCursor(true);
			SetInputMode(FInputModeUIOnly());
		}
	}
	
	if (GetNetMode() == NM_Client)
	{
		return;
	}
	
	UGS_GameInstance* GSInst = GetGameInstance<UGS_GameInstance>();
	
	if (!GSInst)
	{
		return;
	}
	
	if (GSInst->IsLoggedIn())
	{
		GSInst->HostParty(MaxPartyPlayers,NAME_None);
	}
	else
	{
		GSInst->OnGSLoginComplete.AddDynamic(this, &AGS_LobbyPlayerController::OnLoginCompleteForHost);
	}
	
	
}

void AGS_LobbyPlayerController::RequestStartGame()
{
	ServerRequestStartGame();
}

void AGS_LobbyPlayerController::ServerRequestStartGame_Implementation()
{
	if (AGS_LobbyGameMode* GM = GetWorld()->GetAuthGameMode<AGS_LobbyGameMode>())
	{
		GM->TryStartGame(this);
	}
}

void AGS_LobbyPlayerController::OnLoginCompleteForHost(bool bWasSuccessful)
{
	UGS_GameInstance* GSInst = GetGameInstance<UGS_GameInstance>();
	if (!GSInst)
	{
		return;
	}
	
	GSInst->OnGSLoginComplete.RemoveDynamic(this, &AGS_LobbyPlayerController::OnLoginCompleteForHost);
	
	if (bWasSuccessful)
	{
		GSInst->HostParty(MaxPartyPlayers,NAME_None);;
	}
}