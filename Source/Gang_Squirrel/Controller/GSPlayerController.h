#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GSPlayerController.generated.h"

class UGS_GameEndWidget;
class UGS_HUDWidget;
UCLASS()
class GANG_SQUIRREL_API AGSPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	
	UFUNCTION(Client, Reliable)
	void ClientShowGameEndUI();

	UFUNCTION(BlueprintCallable, Category = "Game")
	void RequestRestartGame();

private:
	UFUNCTION(Server, Reliable)
	void ServerSetNickname(const FString& Nickname);

	UFUNCTION(Server, Reliable)
	void ServerRequestRestartGame();

	FTimerHandle MatchEndCheckTimerHandle;

	uint8 bGameEndUIShown : 1 = false;

	void CheckMatchEndByTime();

	void ShowGameEndUILocal();

protected:
	
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UGS_HUDWidget> HUDWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UGS_GameEndWidget> GameEndWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Game")
	FName RestartLevelName = TEXT("/Game/ProjectFile/Level/L_Main_Stage");//Need Lobby Level

	UPROPERTY()
	TObjectPtr<UGS_GameEndWidget> GameEndWidgetInstance;

	UPROPERTY(BlueprintReadOnly, Category = "UI")
	TObjectPtr<UGS_HUDWidget> HUDWidgetInstance;

#pragma region Debugging
public:
	UFUNCTION(Exec)
	void Debug_GiveReward(int32 RewardType);

private:
	UFUNCTION(Server, Reliable)
	void ServerDebugGiveReward(int32 RewardType);

#pragma endregion
};