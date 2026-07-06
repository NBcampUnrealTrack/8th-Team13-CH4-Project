// Fill out your copyright notice in the Description page of Project Settings.


#include "GSPlayerController.h"
#include "Gang_Squirrel/Player/GS_PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "Gang_Squirrel/Game/GS_GameModeBase.h"
#include "Gang_Squirrel/Game/GS_GameState.h"
#include "Gang_Squirrel/UI/GS_GameEndWidget.h"
#include "Gang_Squirrel/UI/GS_NicknameInputWidget.h"
#include "GameFramework/GameStateBase.h"

void AGSPlayerController::BeginPlay()
{
	Super::BeginPlay();

	//UE_LOG(LogTemp, Warning, TEXT("[GSPlayerController] BeginPlay - IsLocal: %d"), IsLocalController());

	if (IsLocalController() == false)
	{
		return;
	}

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

	if (HUDWidgetClass)
	{
		UUserWidget* HUDWidget = CreateWidget<UUserWidget>(this, HUDWidgetClass);
		if (HUDWidget)
		{
			HUDWidget->AddToViewport();
		}
	}
	//UE_LOG(LogTemp, Warning, TEXT("[GSPlayerController] HUD 생성 완료. IsLocal 재확인: %d"), IsLocalController());

	//Skip Nickname 
	if (bSkipNicknameInputForDev)
	{
		const FString DevNickname = FString::Printf(TEXT("Player_%d"), GetLocalPlayer() ? GetLocalPlayer()->GetControllerId() : 0);

		ServerSetNickname(DevNickname);

		SetShowMouseCursor(false);
		SetInputMode(FInputModeGameOnly());

		return;
	}

	if (IsLocalController() && NicknameInputWidgetClass)
	{
		NicknameWidgetInstance = CreateWidget<UGS_NicknameInputWidget>(this, NicknameInputWidgetClass);
		if (IsValid(NicknameWidgetInstance))
		{
			NicknameWidgetInstance->AddToViewport();
			SetShowMouseCursor(true);
			SetInputMode(FInputModeUIOnly());

			//UE_LOG(LogTemp, Warning, TEXT("[GSPlayerController] NicknameWidget 생성 완료. 마우스커서: %d"), bShowMouseCursor);
		}

		//else
		//{
		//	UE_LOG(LogTemp, Warning, TEXT("[GSPlayerController] Widget 생성 실패"));
		//}
	}
}

void AGSPlayerController::SubmitNickname(const FString& Nickname)
{
	//Temp Code
	UE_LOG(LogTemp, Log, TEXT("Nickname: %s"), *Nickname);
	
	//ServerSetNickname(Nickname);
	//SetShowMouseCursor(false);
	//SetInputMode(FInputModeGameOnly());
	if (Nickname.TrimStartAndEnd().IsEmpty())
	{
		if (IsValid(NicknameWidgetInstance))
		{
			NicknameWidgetInstance->ShowError(TEXT("닉네임을 입력해주세요."));
		}
		return;
	}

	ServerSetNickname(Nickname);
}

void AGSPlayerController::ServerSetNickname_Implementation(const FString& Nickname)
{
	//UE_LOG(LogTemp, Warning, TEXT("[ServerSetNickname] 호출됨. Nickname: '%s'"), *Nickname);

	AGS_PlayerState* PS = GetPlayerState<AGS_PlayerState>();
	if (IsValid(PS) == false)
	{
		return;
	}

	if (PS->PlayerNickname.IsEmpty() == false)
	{
		return;
	}

	AGameStateBase* GS = GetWorld()->GetGameState<AGameStateBase>();
	if (IsValid(GS))
	{
		for (APlayerState* OtherPS : GS->PlayerArray)
		{
			AGS_PlayerState* OtherGSPS = Cast<AGS_PlayerState>(OtherPS);
			if (IsValid(OtherGSPS) && OtherGSPS != PS && OtherGSPS->PlayerNickname == Nickname)
			{
				ClientOnNicknameRejected(TEXT("이미 사용중인 닉네임입니다."));
				return;
			}
		}
	}

	PS->SetPlayerNickname(Nickname);
	ClientOnNicknameAccepted();

	AGS_GameModeBase* GM = Cast<AGS_GameModeBase>(GetWorld()->GetAuthGameMode());
	if (IsValid(GM))
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
		UE_LOG(LogTemp, Warning, TEXT("GameEndWidgetClass is nullptr."));
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
		GameEndWidgetInstance->SetGameEndResult(EmptyLeaderboard);

		GameEndWidgetInstance->AddToViewport(100);

		SetShowMouseCursor(true);

		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(GameEndWidgetInstance->TakeWidget());
		SetInputMode(InputMode);
	}
}
void AGSPlayerController::ClientOnNicknameAccepted_Implementation()
{
	if (IsValid(NicknameWidgetInstance))
	{
		NicknameWidgetInstance->RemoveFromParent();
		NicknameWidgetInstance = nullptr;
	}
	SetShowMouseCursor(false);
	SetInputMode(FInputModeGameOnly());
}

void AGSPlayerController::ClientOnNicknameRejected_Implementation(const FString& ErrorMessage)
{
	if (IsValid(NicknameWidgetInstance))
	{
		NicknameWidgetInstance->ShowError(ErrorMessage);
	}
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
