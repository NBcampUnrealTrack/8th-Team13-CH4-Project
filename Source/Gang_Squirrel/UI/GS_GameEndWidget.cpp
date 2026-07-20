#include "GS_GameEndWidget.h"

#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/Button.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Gang_Squirrel/Controller/GSPlayerController.h"
#include "GS_LeaderboardRowWidget.h"
#include "Gang_Squirrel/Player/GS_PlayerState.h"
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

void UGS_GameEndWidget::SetGameEndResultFromData(
	const TArray<FGSLeaderboardEntry>& LeaderboardEntries,
	int32 MyScore
)
{
	UpdateRestartButtonVisibility();

	if (IsValid(TXT_Title))
	{
		TXT_Title->SetText(
			FText::FromString(TEXT("Result"))
		);
	}

	if (IsValid(VB_Leaderboard))
	{
		VB_Leaderboard->ClearChildren();
	}

	if (LeaderboardEntries.IsEmpty())
	{
		if (IsValid(TXT_Winner))
		{
			TXT_Winner->SetText(
				FText::FromString(TEXT("승자 없음"))
			);
		}

		if (IsValid(TXT_MyScore))
		{
			TXT_MyScore->SetText(
				FText::FromString(TEXT("내 점수: 0"))
			);
		}

		if (IsValid(TXT_Notice))
		{
			TXT_Notice->SetText(
				FText::FromString(
					TEXT("리더보드 데이터가 없습니다.")
				)
			);
		}

		return;
	}

	const FGSLeaderboardEntry& WinnerEntry =
		LeaderboardEntries[0];

	if (IsValid(TXT_Winner))
	{
		TXT_Winner->SetText(
			FText::FromString(
				FString::Printf(
					TEXT("%s!"),
					*WinnerEntry.PlayerName
				)
			)
		);
	}

	if (IsValid(TXT_MyScore))
	{
		TXT_MyScore->SetText(
			FText::FromString(
				FString::Printf(
					TEXT("내 점수: %d"),
					MyScore
				)
			)
		);
	}

	if (IsValid(TXT_Notice))
	{
		TXT_Notice->SetText(
			FText::FromString(TEXT("최종 순위"))
		);
	}

	CreateLeaderboardRows(LeaderboardEntries);
}

void UGS_GameEndWidget::CreateLeaderboardRows(const TArray<FGSLeaderboardEntry>& LeaderboardEntries)
{
	if (IsValid(VB_Leaderboard) == false)
	{
		return;
	}

	if (LeaderboardRowWidgetClass == nullptr)
	{
		/*UE_LOG(
			LogTemp,
			Error,
			TEXT(
				"[Leaderboard] LeaderboardRowWidgetClass is nullptr."
			)
		);*/

		return;
	}

	for (const FGSLeaderboardEntry& Entry :
		LeaderboardEntries)
	{
		UGS_LeaderboardRowWidget* RowWidget =
			CreateWidget<UGS_LeaderboardRowWidget>(
				GetOwningPlayer(),
				LeaderboardRowWidgetClass
			);

		if (IsValid(RowWidget) == false)
		{
			continue;
		}

		RowWidget->SetLeaderboardEntry(Entry);
		VB_Leaderboard->AddChild(RowWidget);
	}
}

void UGS_GameEndWidget::UpdateRestartButtonVisibility()
{
	if (IsValid(BTN_RestartGame) == false)
	{
		return;
	}

	APlayerController* OwningPlayer = GetOwningPlayer();
	AGS_PlayerState* MyPlayerState =
		OwningPlayer ? OwningPlayer->GetPlayerState<AGS_PlayerState>() : nullptr;

	const bool bCanRestart = MyPlayerState && MyPlayerState->bIsHost;

	BTN_RestartGame->SetVisibility(
		bCanRestart ? ESlateVisibility::Visible : ESlateVisibility::Collapsed
	);
}

void UGS_GameEndWidget::OnRestartGameClicked()
{
	APlayerController* OwningPlayer = GetOwningPlayer();
	if (OwningPlayer == nullptr)
	{
		return;
	}

	AGSPlayerController* GSPlayerController = Cast<AGSPlayerController>(OwningPlayer);
	if (GSPlayerController == nullptr)
	{
		return;
	}

	GSPlayerController->RequestRestartGame();
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