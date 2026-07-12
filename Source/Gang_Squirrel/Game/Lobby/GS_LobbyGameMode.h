#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GS_LobbyGameMode.generated.h"

UCLASS()
class GANG_SQUIRREL_API AGS_LobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AGS_LobbyGameMode();

protected:
	virtual void PostLogin(APlayerController* NewPlayer) override;
	
public:
	UFUNCTION(BlueprintCallable,Category="Lobby")
	void TryStartGame(APlayerController* Requester);
	
	UFUNCTION(BlueprintPure,Category="Lobby")
	bool CanStartGame();
	
protected:
	UPROPERTY(EditDefaultsOnly,Category="Lobby")
	int32 MinPlayersToStart = 2;
	UPROPERTY(EditDefaultsOnly,Category="Lobby")
	FName MainStageLevelName = "/Game/ProjectFile/Level/L_Main_Stage";
	
};
