// Fill out your copyright notice in the Description page of Project Settings.


#include "Gang_Squirrel/Controller/GS_StartMenuPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Gang_Squirrel/Game/GS_StartMenu_GameMode.h"

void AGS_StartMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController() == false)
	{
		return;
	}

	if (StartMenuWidgetClass)
	{
		UUserWidget* StartMenuWidget = CreateWidget<UUserWidget>(this, StartMenuWidgetClass);
		if (StartMenuWidget)
		{
			StartMenuWidget->AddToViewport();
			SetShowMouseCursor(true);
			SetInputMode(FInputModeUIOnly());
		}
	}
	
}

void AGS_StartMenuPlayerController::RequestLobbyReady()
{
	ServerRequestLobbyReady();
}

void AGS_StartMenuPlayerController::ServerRequestLobbyReady_Implementation()
{
	AGS_StartMenu_GameMode* GM = Cast<AGS_StartMenu_GameMode>(GetWorld()->GetAuthGameMode());
	if (IsValid(GM))
	{
		GM->OnLobbyPlayerReady();
	}
}
