#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_LobbyWidget.generated.h"

class UGS_FriendListWidget;
class UButton;
class UGS_LobbySlotWidget;
class UPanelWidget;
class UGS_SettingWidget;

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
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> Button_Invite;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPanelWidget> FriendListContainer;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Settings;

	
	UPROPERTY(EditDefaultsOnly,Category="UI")
	TSubclassOf<UGS_LobbySlotWidget> SlotWidgetClass;
	UPROPERTY(EditDefaultsOnly,Category="UI")
	TSubclassOf<UGS_FriendListWidget> FriendWidgetClass;
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UGS_SettingWidget> SettingWidgetClass;
	
	UPROPERTY(EditDefaultsOnly,Category="UI")
	int32 MaxLobbySize = 4;
	UPROPERTY(EditDefaultsOnly,Category="Lobby")
	int32 MinPlayersToStart = 2;
	
private:
	UPROPERTY()
	TArray<TObjectPtr<UGS_LobbySlotWidget>> Slots;
	UPROPERTY()
	TObjectPtr<UGS_FriendListWidget> FriendListWidgetInst;
	UPROPERTY()
	TObjectPtr<UGS_SettingWidget> SettingWidgetInst;
	
	FTimerHandle RefreshTimerHandle;
	
	void CreateSlotPool();
	void RefreshLobby();
	void RefreshStartButton();
	
	UFUNCTION()
	void OnStartButtonClicked();
	UFUNCTION()
	void OnInviteButtonClicked();
	UFUNCTION()
	void OnSettingsButtonClicked();
	
	void ToggleFriendList();
};
