#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GSPlayerController.generated.h"

class UGS_GameEndWidget;
class UGS_NicknameInputWidget;

UCLASS()
class GANG_SQUIRREL_API AGSPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	// Call after entering nickname
	UFUNCTION(BlueprintCallable, Category = "Player")
	void SubmitNickname(const FString& Nickname);

	UFUNCTION(Client, Reliable)
	void ClientShowGameEndUI();

	UFUNCTION(Client, Reliable)
	void ClientOnNicknameAccepted();

	UFUNCTION(Client, Reliable)
	void ClientOnNicknameRejected(const FString& ErrorMessage);

private:
	UFUNCTION(Server, Reliable)
	void ServerSetNickname(const FString& Nickname);

	FTimerHandle MatchEndCheckTimerHandle;

	uint8 bGameEndUIShown : 1 = false;

	void CheckMatchEndByTime();

	void ShowGameEndUILocal();


protected:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UGS_NicknameInputWidget> NicknameInputWidgetClass;

	UPROPERTY()
	TObjectPtr<UGS_NicknameInputWidget> NicknameWidgetInstance;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> HUDWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UGS_GameEndWidget> GameEndWidgetClass;

	UPROPERTY()
	TObjectPtr<UGS_GameEndWidget> GameEndWidgetInstance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DEV")
	uint8 bSkipNicknameInputForDev : 1 = false;
};