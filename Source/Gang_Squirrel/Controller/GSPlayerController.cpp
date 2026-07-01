// Fill out your copyright notice in the Description page of Project Settings.


#include "GSPlayerController.h"
#include "Gang_Squirrel/Player/GS_PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "Gang_Squirrel/Game/GS_GameModeBase.h"

void AGSPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController() == false)
	{
		return;
	}

	FInputModeGameOnly IMGameOnly;
	SetInputMode(IMGameOnly);

	if (HUDWidgetClass)
	{
		UUserWidget* HUDWidget = CreateWidget<UUserWidget>(this, HUDWidgetClass);
		if (HUDWidget)
		{
			HUDWidget->AddToViewport();
		}
	}

	if (IsLocalController() && NicknameInputWidgetClass)
	{
		UUserWidget* Widget = CreateWidget<UUserWidget>(this, NicknameInputWidgetClass);
		if (IsValid(Widget))
		{
			Widget->AddToViewport();
			SetShowMouseCursor(true);
			SetInputMode(FInputModeUIOnly());
		}
	}
}

void AGSPlayerController::SubmitNickname(const FString& Nickname)
{
	//Temp Code
	UE_LOG(LogTemp, Log, TEXT("Nickname: %s"), *Nickname);
	
	ServerSetNickname(Nickname);

	SetShowMouseCursor(false);
	SetInputMode(FInputModeGameOnly());
}

void AGSPlayerController::ServerSetNickname_Implementation(const FString& Nickname)
{
	AGS_PlayerState* PS = GetPlayerState<AGS_PlayerState>();

	if (PS->PlayerNickname.IsEmpty() == false)
	{
		return;
	}

	if (IsValid(PS))
	{
		PS->SetPlayerNickname(Nickname);
	}

	//Cast to server GameMode
	AGS_GameModeBase* GM = Cast<AGS_GameModeBase>(GetWorld()->GetAuthGameMode());
	if (IsValid(GM))
	{
		GM->NotifyPlayerReady();
	}
}
