#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GSPlayerController.generated.h"

class UGS_GameEndWidget;
class AGSCharacter;
class UGS_HUDWidget;
class UInputMappingContext;
class UInputAction;

UCLASS()
class GANG_SQUIRREL_API AGSPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	//UFUNCTION(Client, Reliable)
	//void ClientShowGameEndUI();

	UFUNCTION(Client, Reliable)
	void ClientShowResultStage();

	UFUNCTION(BlueprintCallable, Category = "Game")
	void RequestRestartGame();

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UFUNCTION(Server, Reliable)
	void ServerSetNickname(const FString& Nickname);

	UFUNCTION(Server, Reliable)
	void ServerRequestRestartGame();

	//FTimerHandle MatchEndCheckTimerHandle;
	//
	//uint8 bGameEndUIShown : 1 = false;
	//
	//void CheckMatchEndByTime();
	//
	//void ShowGameEndUILocal();

protected:
	
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UGS_HUDWidget> HUDWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UGS_GameEndWidget> GameEndWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Game")
	FName RestartLevelName = TEXT("/Game/ProjectFile/Level/Lobby/L_Lobby");//Need Lobby Level

	UPROPERTY()
	TObjectPtr<UGS_GameEndWidget> GameEndWidgetInstance;

	UPROPERTY(BlueprintReadOnly, Category = "UI")
	TObjectPtr<UGS_HUDWidget> HUDWidgetInstance;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> IMC_UI;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_ToggleSettings;

public:
	void HandleToggleSettings();

private:
	FTimerHandle ResultDataRetryTimerHandle;

	int32 ResultDataRetryCount = 0;

	void TrySetGameEndResult();

#pragma region Debugging
public:
	UFUNCTION(Exec)
	void Debug_GiveReward(int32 RewardType);

private:
	UFUNCTION(Server, Reliable)
	void ServerDebugGiveReward(int32 RewardType);

#pragma endregion

#pragma region Result Level

protected:
	UPROPERTY(EditDefaultsOnly, Category = "GameEnd|Stage")
	FName VictoryStageCameraTag = "VictoryStageCamera";

#pragma endregion
};