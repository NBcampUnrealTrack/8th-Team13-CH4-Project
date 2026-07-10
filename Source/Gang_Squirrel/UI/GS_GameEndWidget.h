#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_GameEndWidget.generated.h"

class UTextBlock;
class UVerticalBox;
class UButton;
class UGS_LeaderboardRowWidget;

USTRUCT(BlueprintType)
struct FGSLeaderboardEntry
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "Leaderboard")
	int32 Rank = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Leaderboard")
	FString PlayerName;

	UPROPERTY(BlueprintReadOnly, Category = "Leaderboard")
	int32 Score = 0;
};

UCLASS()
class GANG_SQUIRREL_API UGS_GameEndWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category = "Game End")
	void SetGameEndResult();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TXT_Title;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TXT_Winner;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TXT_MyScore;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TXT_Notice;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> VB_Leaderboard;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BTN_RestartGame;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BTN_QuitGame;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Leaderboard")
	TSubclassOf<UGS_LeaderboardRowWidget> LeaderboardRowWidgetClass;

private:

	void CreateLeaderboardRows(
		const TArray<FGSLeaderboardEntry>& LeaderboardEntries
	);

	UFUNCTION()
	void OnRestartGameClicked();

	UFUNCTION()
	void OnQuitGameClicked();
};