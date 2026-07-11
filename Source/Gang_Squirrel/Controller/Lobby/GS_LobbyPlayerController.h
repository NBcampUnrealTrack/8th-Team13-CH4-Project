#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GS_LobbyPlayerController.generated.h"

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
	
public:
	UFUNCTION(BlueprintCallable,Category="Lobby")
	void RequestStartGame();
	
private:
	UFUNCTION(Server,Reliable)
	void ServerRequestStartGame();
	UFUNCTION()
	void OnLoginCompleteForHost(bool bWasSuccessful);
};
