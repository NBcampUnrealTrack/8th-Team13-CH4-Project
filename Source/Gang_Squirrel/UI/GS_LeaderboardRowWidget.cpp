#include "GS_LeaderboardRowWidget.h"

#include "Components/TextBlock.h"

void UGS_LeaderboardRowWidget::SetLeaderboardEntry(const FGSLeaderboardEntry& InEntry)
{
	if (IsValid(TXT_Rank))
	{
		TXT_Rank->SetText(
			FText::FromString(
				FString::Printf(TEXT("%d"), InEntry.Rank)
			)
		);
	}

	if (IsValid(TXT_PlayerName))
	{
		TXT_PlayerName->SetText(FText::FromString(InEntry.PlayerName));
	}

	if (IsValid(TXT_Score))
	{
		TXT_Score->SetText(
			FText::FromString(
				FString::Printf(TEXT("%d"), InEntry.Score)
			)
		);
	}
}