#include "GS_GameEndWidget.h"

#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/Button.h"
#include "GS_LeaderboardRowWidget.h"
#include "Gang_Squirrel/Player/GS_PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void UGS_GameEndWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (IsValid(BTN_RestartGame))
	{
		BTN_RestartGame->OnClicked.AddDynamic(this, &ThisClass::OnRestartGameClicked);
	}

	if (IsValid(BTN_QuitGame))
	{
		BTN_QuitGame->OnClicked.AddDynamic(this, &ThisClass::OnQuitGameClicked);
	}
}

void UGS_GameEndWidget::SetGameEndResult(const TArray<FGSLeaderboardEntry>& InLeaderboardEntries)
{
	if (IsValid(TXT_Title))
	{
		TXT_Title->SetText(FText::FromString(TEXT("Result")));
	}
	
	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		AGS_PlayerState* PS = PC->GetPlayerState<AGS_PlayerState>();
		
		if (PS && TXT_MyScore)
		{
			TXT_MyScore->SetText(FText::FromString(FString::Printf(TEXT("내 점수 : %d"), PS->GetPlayerScore())));
		}
	}

	if (IsValid(VB_Leaderboard))
	{
		VB_Leaderboard->ClearChildren();
	}

	if (InLeaderboardEntries.Num() <= 0)
	{
		if (IsValid(TXT_Winner))
		{
			TXT_Winner->SetText(FText::FromString(TEXT("승자 집계 예정")));
		}

		if (IsValid(TXT_MyScore))
		{
			TXT_MyScore->SetText(FText::FromString(TEXT("내 점수: 준비 중")));
		}

		if (IsValid(TXT_Notice))
		{
			TXT_Notice->SetText(FText::FromString(TEXT("리더보드는 점수 시스템 추가 후 표시됩니다.")));
		}

		return;
	}

	const FGSLeaderboardEntry& WinnerEntry = InLeaderboardEntries[0];

	if (IsValid(TXT_Winner))
	{
		TXT_Winner->SetText(
			FText::FromString(
				FString::Printf(TEXT("승자: %s"), *WinnerEntry.PlayerName)
			)
		);
	}

	if (IsValid(TXT_Notice))
	{
		TXT_Notice->SetText(FText::FromString(TEXT("최종 순위")));
	}

	if (IsValid(VB_Leaderboard) == false)
	{
		return;
	}

	if (LeaderboardRowWidgetClass == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("LeaderboardRowWidgetClass is nullptr."));
		return;
	}

	for (const FGSLeaderboardEntry& Entry : InLeaderboardEntries)
	{
		UGS_LeaderboardRowWidget* RowWidget = CreateWidget<UGS_LeaderboardRowWidget>(
			GetOwningPlayer(),
			LeaderboardRowWidgetClass
		);

		if (IsValid(RowWidget))
		{
			RowWidget->SetLeaderboardEntry(Entry);
			VB_Leaderboard->AddChild(RowWidget);
		}
	}
}

void UGS_GameEndWidget::OnRestartGameClicked()
{
	UWorld* World = GetWorld();
	if (IsValid(World) == false)
	{
		return;
	}

	const FName CurrentLevelName = FName(*UGameplayStatics::GetCurrentLevelName(World));

	UGameplayStatics::OpenLevel(
		World,
		CurrentLevelName
	);
}

void UGS_GameEndWidget::OnQuitGameClicked()
{
	APlayerController* OwningPlayer = GetOwningPlayer();

	UKismetSystemLibrary::QuitGame(
		this,
		OwningPlayer,
		EQuitPreference::Quit,
		false
	);
}