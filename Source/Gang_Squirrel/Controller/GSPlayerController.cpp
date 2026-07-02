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
	//Skip Nickname 
	if (bSkipNicknameInputForDev)
	{
		const FString DevNickname = FString::Printf(TEXT("Player_%d"), GetLocalPlayer() ? GetLocalPlayer()->GetControllerId() : 0);

		ServerSetNickname(DevNickname);

		SetShowMouseCursor(false);
		SetInputMode(FInputModeGameOnly());

		return;
	}

	if (NicknameInputWidgetClass)
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
	if (IsValid(PS) == false)
	{
		return;
	}

	if (PS->PlayerNickname.IsEmpty() == false)
	{
		return;
	}

	PS->SetPlayerNickname(Nickname);

	AGS_GameModeBase* GM = Cast<AGS_GameModeBase>(GetWorld()->GetAuthGameMode());
	if (IsValid(GM))
	{
		GM->NotifyPlayerReady();
	}
}
void AGSPlayerController::ClientShowGameEndUI_Implementation()
{
	if (IsLocalController() == false)
	{
		return;
	}

	if (GameEndWidgetClass == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameEndWidgetClass is nullptr."));
		return;
	}

	if (GameEndWidgetInstance == nullptr)
	{
		GameEndWidgetInstance = CreateWidget<UUserWidget>(this, GameEndWidgetClass);
	}

	if (IsValid(GameEndWidgetInstance))
	{
		GameEndWidgetInstance->AddToViewport(100);

		SetShowMouseCursor(true);

		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(GameEndWidgetInstance->TakeWidget());
		SetInputMode(InputMode);
	}
}