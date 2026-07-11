#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_LobbyWidget.generated.h"

class UGS_LobbySlotWidget;

UCLASS()
class GANG_SQUIRREL_API UGS_LobbyWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	UFUNCTION(BlueprintPure,Category="Lobby")
	bool IsLocalPlayerHost() const;
	UFUNCTION(BlueprintPure,Category="Lobby")
	bool CanStartGame() const;
	
protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPanelWidget> SlotContainer;
	
	UPROPERTY(EditDefaultsOnly,Category="UI")
	TSubclassOf<UGS_LobbySlotWidget> SlotWidgetClass;
	
	UPROPERTY(EditDefaultsOnly,Category="UI")
	int32 MaxLobbySize = 4;
	UPROPERTY(EditDefaultsOnly,Category="Lobby")
	int32 MinPlayersToStart = 2;
	
private:
	UPROPERTY()
	TArray<TObjectPtr<UGS_LobbySlotWidget>> Slots;
	
	FTimerHandle RefreshTimerHanlde;
	
	void CreateSlotPool();
	void RefreshLobby();
};
