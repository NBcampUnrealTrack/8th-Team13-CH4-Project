#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GSPlayerController.generated.h"

UCLASS()
class GANG_SQUIRREL_API AGSPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	// Call after entering nickname
	UFUNCTION(BlueprintCallable, Category = "Player")
	void SubmitNickname(const FString& Nickname);

	// 서버가 게임 종료 시 각 클라이언트에게 호출한다.
	UFUNCTION(Client, Reliable)
	void ClientShowGameEndUI();

private:
	UFUNCTION(Server, Reliable)
	void ServerSetNickname(const FString& Nickname);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> NicknameInputWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> HUDWidgetClass;

	// 게임 종료 UI
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> GameEndWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> GameEndWidgetInstance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DEV")
	uint8 bSkipNicknameInputForDev : 1 = false;
};