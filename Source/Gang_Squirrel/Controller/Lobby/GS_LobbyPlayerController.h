#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GS_LobbyPlayerController.generated.h"

class AGSCharacter;
class UUserWidget;
class AGSCharacter;
class UInputMappingContext;
class UInputAction;

UCLASS()
class GANG_SQUIRREL_API AGS_LobbyPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:
	UPROPERTY(EditDefaultsOnly,Category="UI")
	TSubclassOf<UUserWidget> LobbyWidgetClass;
	
	UPROPERTY(EditDefaultsOnly,Category="Lobby")
	int32 MaxPartyPlayers = 4;
	
	UPROPERTY(EditDefaultsOnly,Category="Lobby|Display")
	TSubclassOf<AGSCharacter> DisplayCharacterClass;
	UPROPERTY(EditDefaultsOnly,Category="Lobby|Display")
	FName LobbyCameraTag = "LobbyCamera";

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> IMC_UI;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_ToggleSettings;

public:
	// GS_SettingWidget이 ESC-in-widget / 닫기버튼으로 자체 종료할 때도
	// 재사용할 수 있도록 public으로 노출
	void HandleToggleSettings();


	UFUNCTION(BlueprintCallable,Category="Lobby")
	void RequestStartGame();

	UFUNCTION(Client, Reliable)
	void ClientShowLoadingWidget();

private:
	UFUNCTION(Server,Reliable)
	void ServerRequestStartGame();

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> LoadingWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> LoadingWidgetInstance;

#pragma region ReadySystem
	
public:
	UFUNCTION(BlueprintCallable,Category="Lobby")
	void RequestToggleReady();
private:
	UFUNCTION(Server,Reliable)
	void ServerToggleReady();
	
#pragma endregion 
	
	UFUNCTION()
	void OnLoginCompleteForHost(bool bWasSuccessful);
	UFUNCTION(Server,Reliable)
	void ServerSetNickname(const FString& Nickname);
	
	void SetupLobbyCamera();
	void CacheCharacterSlots();
	void RefreshCharacterDisplay();
	
	UPROPERTY()
	TArray<FTransform> SlotTransforms;
	UPROPERTY()
	TArray<TObjectPtr<AGSCharacter>> DisplayCharacters;
	
	FTimerHandle CharacterDisplayTimerHandle;

	void ShowLoadingWidget();
};
