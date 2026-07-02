// Fill out your copyright notice in the Description page of Project Settings.


#include "Gang_Squirrel/Controller/GS_StartMenuPlayerController.h"
#include "Blueprint/UserWidget.h"

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
