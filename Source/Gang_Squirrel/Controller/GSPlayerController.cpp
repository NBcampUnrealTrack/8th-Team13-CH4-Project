// Fill out your copyright notice in the Description page of Project Settings.


#include "GSPlayerController.h"
#include "Gang_Squirrel/Player/GS_PlayerState.h"
#include "Blueprint/UserWidget.h"
#include "Gang_Squirrel/Game/GS_GameModeBase.h"
#include "Gang_Squirrel/Game/GS_GameState.h"
#include "Gang_Squirrel/UI/GS_GameEndWidget.h"
#include "Gang_Squirrel/UI/HUD/GS_HUDWidget.h"
#include "GameFramework/GameStateBase.h"
#include "Gang_Squirrel/EOS/GS_GameInstance.h"
#include "Gang_Squirrel/Character/GSCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameViewportClient.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

void AGSPlayerController::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("[GSPlayerController] BeginPlay - IsLocal: %d"), IsLocalController());

	if (IsLocalController() == false)
	{
		return;
	}

	if (UGS_GameInstance* GSInstance =
		GetGameInstance<UGS_GameInstance>())
	{
		GSInstance->StopLoadingScreen();
	}

	//로비에서 설정한 밝기값을 메인스테이지에 가져오는 코드
	if (UGS_GameInstance* GSInst = GetGameInstance<UGS_GameInstance>())
	{
		GSInst->SetScreenBrightness(GSInst->ScreenBrightness);
	}

	TArray<AActor*> StageCameras;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), VictoryStageCameraTag, StageCameras);
	if (StageCameras.Num() > 0)
	{
		return;   // Result 레벨의 UI/카메라/캐릭터는 ClientShowResultStage(RPC)가 전담
	}
	// bGameEndUIShown = false;

	//if (IsValid(GameEndWidgetInstance))
	//{
	//	GameEndWidgetInstance->RemoveFromParent();
	//	GameEndWidgetInstance = nullptr;
	//}

	//GetWorldTimerManager().ClearTimer(MatchEndCheckTimerHandle);
	//
	//GetWorldTimerManager().SetTimer(
	//	MatchEndCheckTimerHandle,
	//	this,
	//	&AGSPlayerController::CheckMatchEndByTime,
	//	0.2f,
	//	true
	//);

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
		HUDWidgetInstance = CreateWidget<UGS_HUDWidget>(this, HUDWidgetClass);
		
		if (IsValid(HUDWidgetInstance))
		{
			HUDWidgetInstance->AddToViewport();
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

void AGSPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (!IsLocalController())
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(IMC_UI, 1);
	}

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EIC->BindAction(IA_ToggleSettings, ETriggerEvent::Started, this, &AGSPlayerController::HandleToggleSettings);
	}
}

void AGSPlayerController::HandleToggleSettings()
{
	UGS_GameInstance* GSInst = GetGameInstance<UGS_GameInstance>();
	if (!GSInst)
	{
		return;
	}

	if (GSInst->ToggleSettingsWidget(this))
	{
		SetShowMouseCursor(true);
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
	}
	else
	{
		SetShowMouseCursor(false);
		SetInputMode(FInputModeGameOnly());
	}
}

void AGSPlayerController::ClientShowResultStage_Implementation()
{
	//UE_LOG(LogTemp, Warning, TEXT("[Result] GameEndWidgetClass=%s"),GameEndWidgetClass ? TEXT("Valid") : TEXT("NULL"));

	if (UGameViewportClient* ViewportClient = GetWorld() ? GetWorld()->GetGameViewport() : nullptr)
	{
		ViewportClient->RemoveAllViewportWidgets();
	}

	if (GameEndWidgetClass)
	{
		GameEndWidgetInstance = CreateWidget<UGS_GameEndWidget>(this, GameEndWidgetClass);

		//UE_LOG(LogTemp, Warning, TEXT("[Result] GameEndWidgetInstance=%s"),IsValid(GameEndWidgetInstance) ? TEXT("Valid") : TEXT("NULL"));

		if (IsValid(GameEndWidgetInstance))
		{
			GameEndWidgetInstance->SetGameEndResult();
			GameEndWidgetInstance->AddToViewport(100);

			//UE_LOG(LogTemp, Warning, TEXT("[Result] AddToViewport called"));

			SetShowMouseCursor(true);
			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(GameEndWidgetInstance->TakeWidget());
			SetInputMode(InputMode);
		}
	}

	TArray<AActor*> FoundCameras;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), VictoryStageCameraTag, FoundCameras);

	if (FoundCameras.Num() > 0)
	{
		SetViewTargetWithBlend(FoundCameras[0], 0.5f);
	}
}

void AGSPlayerController::RequestRestartGame()
{
	ServerRequestRestartGame();
}

void AGSPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsValid(HUDWidgetInstance))
	{
		HUDWidgetInstance->RemoveFromParent();
		HUDWidgetInstance = nullptr;
	}

	if (IsValid(GameEndWidgetInstance))
	{
		GameEndWidgetInstance->RemoveFromParent();
		GameEndWidgetInstance = nullptr;
	}

	Super::EndPlay(EndPlayReason);
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

//void AGSPlayerController::ClientShowGameEndUI_Implementation()
//{
//	ShowGameEndUILocal();
//}

//void AGSPlayerController::ShowGameEndUILocal()
//{
//	if (IsLocalController() == false)
//	{
//		return;
//	}
//
//	if (GameEndWidgetClass == nullptr)
//	{
//		// UE_LOG(LogTemp, Warning, TEXT("GameEndWidgetClass is nullptr."));
//		return;
//	}
//
//	if (GameEndWidgetInstance == nullptr)
//	{
//		GameEndWidgetInstance = CreateWidget<UGS_GameEndWidget>(
//			this,
//			GameEndWidgetClass
//		);
//	}
//
//	if (IsValid(GameEndWidgetInstance))
//	{
//		TArray<FGSLeaderboardEntry> EmptyLeaderboard;
//		GameEndWidgetInstance->SetGameEndResult();
//
//		GameEndWidgetInstance->AddToViewport(100);
//
//		SetShowMouseCursor(true);
//
//		FInputModeUIOnly InputMode;
//		InputMode.SetWidgetToFocus(GameEndWidgetInstance->TakeWidget());
//		SetInputMode(InputMode);
//	}
//}

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

//void AGSPlayerController::CheckMatchEndByTime()
//{
//	if (bGameEndUIShown)
//	{
//		return;
//	}
//
//	AGS_GameState* GS = GetWorld() ? GetWorld()->GetGameState<AGS_GameState>() : nullptr;
//	if (IsValid(GS) == false)
//	{
//		return;
//	}
//
//	if (GS->MatchEndTime <= 0.f)
//	{
//		return;
//	}
//
//	if (GS->GetRemainingTime() > 0.f)
//	{
//		return;
//	}
//
//	bGameEndUIShown = true;
//
//	GetWorldTimerManager().ClearTimer(MatchEndCheckTimerHandle);
//
//	ShowGameEndUILocal();
//}
