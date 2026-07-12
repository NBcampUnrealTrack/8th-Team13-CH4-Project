// Fill out your copyright notice in the Description page of Project Settings.


#include "GSPlayerController.h"
#include "Gang_Squirrel/Player/GS_PlayerState.h"
#include "Blueprint/UserWidget.h"
#include "Gang_Squirrel/Game/GS_GameModeBase.h"
#include "Gang_Squirrel/Game/GS_GameState.h"
#include "Gang_Squirrel/UI/GS_GameEndWidget.h"
#include "GameFramework/GameStateBase.h"
#include "Gang_Squirrel/EOS/GS_GameInstance.h"

void AGSPlayerController::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("[GSPlayerController] BeginPlay - IsLocal: %d"), IsLocalController());

	if (IsLocalController() == false)
	{
		return;
	}

	bGameEndUIShown = false;

	if (IsValid(GameEndWidgetInstance))
	{
		GameEndWidgetInstance->RemoveFromParent();
		GameEndWidgetInstance = nullptr;
	}

	GetWorldTimerManager().ClearTimer(MatchEndCheckTimerHandle);

	GetWorldTimerManager().SetTimer(
		MatchEndCheckTimerHandle,
		this,
		&AGSPlayerController::CheckMatchEndByTime,
		0.2f,
		true
	);

	//UE_LOG(LogTemp, Warning, TEXT("[GSPlayerController] HUDClass: %d / NicknameClass: %d"),
	//	HUDWidgetClass != nullptr,
	//	NicknameInputWidgetClass != nullptr);

	FInputModeGameOnly IMGameOnly;
	SetInputMode(IMGameOnly);

	if (IsValid(HUDWidgetInstance))
	{
		HUDWidgetInstance->RemoveFromParent();
		HUDWidgetInstance = nullptr;
	}

	if (HUDWidgetClass)
	{
		UUserWidget* HUDWidget = CreateWidget<UUserWidget>(this, HUDWidgetClass);
		if (HUDWidget)
		{
			HUDWidget->AddToViewport();
		}
	}
	
	FString DisplayName;
	if (UGS_GameInstance* GSInst = GetGameInstance<UGS_GameInstance>())
	{
		DisplayName = GSInst->GetLocalDisplayName();
	}
	
	if (DisplayName.IsEmpty())
	{
		DisplayName = FString::Printf(TEXT("Player %d"),GetLocalPlayer() ? GetLocalPlayer()->GetControllerId() : 0);
	}
	
	ServerSetNickname(DisplayName);
}

void AGSPlayerController::RequestRestartGame()
{
	ServerRequestRestartGame();
}

void AGSPlayerController::ServerRequestRestartGame_Implementation()
{
	AGS_PlayerState* GSPS = GetPlayerState<AGS_PlayerState>();
	if (GSPS == nullptr || GSPS->bIsHost == false)
	{
		return;
	}

	UGS_GameInstance* GSInst = GetGameInstance<UGS_GameInstance>();
	if (GSInst == nullptr)
	{
		return;
	}

	GSInst->StartGame(RestartLevelName);
}

void AGSPlayerController::ServerSetNickname_Implementation(const FString& Nickname)
{
	// UE_LOG(LogTemp, Warning, TEXT("[ServerSetNickname] 호출됨. Nickname: '%s'"), *Nickname);

	AGS_PlayerState* PS = GetPlayerState<AGS_PlayerState>();
	if (IsValid(PS) == false)
	{
		return;
	}

	if (PS->PlayerNickname.IsEmpty() == false)
	{
		return;
	}

	FString FinalNickName = Nickname;
	
	AGameStateBase* GS = GetWorld()->GetGameState<AGameStateBase>();
	if (GS)
	{
		int32 Suffix = 2;
		bool bDuplicate = true;
		while (bDuplicate)
		{
			bDuplicate = false;
			for (APlayerState* OtherPS : GS->PlayerArray)
			{
				AGS_PlayerState* OtherGSPS = Cast<AGS_PlayerState>(OtherPS);
				if (OtherGSPS && OtherGSPS != PS && OtherGSPS->PlayerNickname == FinalNickName)
				{
					FinalNickName = FString::Printf(TEXT("%s(%d)"),*Nickname,Suffix++);
					bDuplicate = true;
					break;
				}
			}
		}
	}
	
	PS->SetPlayerNickname(FinalNickName);
	
	AGS_GameModeBase* GM = Cast<AGS_GameModeBase>(GetWorld()->GetAuthGameMode());
	if (GM)
	{
		GM->NotifyPlayerReady();
	}
}

void AGSPlayerController::ClientShowGameEndUI_Implementation()
{
	ShowGameEndUILocal();
}

void AGSPlayerController::ShowGameEndUILocal()
{
	if (IsLocalController() == false)
	{
		return;
	}

	if (GameEndWidgetClass == nullptr)
	{
		// UE_LOG(LogTemp, Warning, TEXT("GameEndWidgetClass is nullptr."));
		return;
	}

	if (GameEndWidgetInstance == nullptr)
	{
		GameEndWidgetInstance = CreateWidget<UGS_GameEndWidget>(
			this,
			GameEndWidgetClass
		);
	}

	if (IsValid(GameEndWidgetInstance))
	{
		TArray<FGSLeaderboardEntry> EmptyLeaderboard;
		GameEndWidgetInstance->SetGameEndResult();

		GameEndWidgetInstance->AddToViewport(100);

		SetShowMouseCursor(true);

		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(GameEndWidgetInstance->TakeWidget());
		SetInputMode(InputMode);
	}
}

void AGSPlayerController::Debug_GiveReward(int32 RewardType)
{
	ServerDebugGiveReward(RewardType);
}

void AGSPlayerController::ServerDebugGiveReward_Implementation(int32 RewardType)
{
	AGS_PlayerState* PS = GetPlayerState<AGS_PlayerState>();
	if (!IsValid(PS)) return;

	AGS_GameModeBase* GM = Cast<AGS_GameModeBase>(GetWorld()->GetAuthGameMode());
	if (!IsValid(GM)) return;

	if (RewardType < 0 || RewardType > 2)
	{
		// UE_LOG(LogTemp, Warning, TEXT("[Debug] Invalid RewardType: %d (0=Food, 1=Capacity, 2=SpeedBoost)"), RewardType);
		return;
	}

	GM->GiveSpecificReward(PS, static_cast<ERewardType>(RewardType));

//	UE_LOG(LogTemp, Log, TEXT("[Debug] GiveSpecificReward called: %d"), RewardType);
}

void AGSPlayerController::CheckMatchEndByTime()
{
	if (bGameEndUIShown)
	{
		return;
	}

	AGS_GameState* GS = GetWorld() ? GetWorld()->GetGameState<AGS_GameState>() : nullptr;
	if (IsValid(GS) == false)
	{
		return;
	}

	if (GS->MatchEndTime <= 0.f)
	{
		return;
	}

	if (GS->GetRemainingTime() > 0.f)
	{
		return;
	}

	bGameEndUIShown = true;

	GetWorldTimerManager().ClearTimer(MatchEndCheckTimerHandle);

	ShowGameEndUILocal();
}
