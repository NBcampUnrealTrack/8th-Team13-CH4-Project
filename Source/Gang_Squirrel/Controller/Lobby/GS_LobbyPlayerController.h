#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GS_LobbyPlayerController.generated.h"

class AGSCharacter;
class UUserWidget;
class AGSCharacter;

UCLASS()
class GANG_SQUIRREL_API AGS_LobbyPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;
	
private:
	UPROPERTY(EditDefaultsOnly,Category="UI")
	TSubclassOf<UUserWidget> LobbyWidgetClass;
	
	UPROPERTY(EditDefaultsOnly,Category="Lobby")
	int32 MaxPartyPlayers = 4;
	
	UPROPERTY(EditDefaultsOnly,Category="Lobby|Display")
	TSubclassOf<AGSCharacter> DisplayCharacterClass;
	UPROPERTY(EditDefaultsOnly,Category="Lobby|Display")
	FName LobbyCameraTag = "LobbyCamera";
	
public:
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
