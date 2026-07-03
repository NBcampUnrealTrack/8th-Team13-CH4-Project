#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_GameEndWidget.h"
#include "GS_LeaderboardRowWidget.generated.h"

class UTextBlock;


UCLASS()
class GANG_SQUIRREL_API UGS_LeaderboardRowWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Leaderboard")
	void SetLeaderboardEntry(const FGSLeaderboardEntry& InEntry);

protected:
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TXT_Rank;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TXT_PlayerName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TXT_Score;
};