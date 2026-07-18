#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GS_LobbyPlayerController.generated.h"

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

	void HandleToggleSettings();

public:
	UFUNCTION(BlueprintCallable,Category="Lobby")
	void RequestStartGame();
	
	UFUNCTION(Client, Reliable)
	void ClientStartLoadingScreen();

private:
	UFUNCTION(Server,Reliable)
	void ServerRequestStartGame();
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
};
