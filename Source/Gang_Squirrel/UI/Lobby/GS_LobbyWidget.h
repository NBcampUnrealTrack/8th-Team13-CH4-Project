#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_LobbyWidget.generated.h"

class UButton;
class UGS_LobbySlotWidget;
class UPanelWidget;

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
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> Button_Start;
	
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
	void RefreshStartButton();
	
	UFUNCTION()
	void OnStartButtonClicked();
};
