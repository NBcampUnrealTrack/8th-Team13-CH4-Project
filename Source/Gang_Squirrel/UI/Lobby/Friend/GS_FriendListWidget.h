#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_FriendListWidget.generated.h"


class UButton;
class UGS_FriendEntryWidget;
class UPanelWidget;

UCLASS()
class GANG_SQUIRREL_API UGS_FriendListWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPanelWidget> EntryContainer;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> Button_Exit;
	
	UPROPERTY(EditDefaultsOnly,Category="UI")
	TSubclassOf<UGS_FriendEntryWidget> EntryWidgetClass;
	
private:
	UFUNCTION()
	void OnFriendsListComplete(bool bWasSuccessful);
	UFUNCTION()
	void OnExitButtonClicked();
	
	void RefreshEntries();
};
