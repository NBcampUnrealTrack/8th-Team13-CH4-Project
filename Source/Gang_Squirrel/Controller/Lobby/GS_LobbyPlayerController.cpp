#include "GS_LobbyPlayerController.h"

#include "Blueprint/UserWidget.h"
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
