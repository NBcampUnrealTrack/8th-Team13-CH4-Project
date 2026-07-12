#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_LobbySlotWidget.generated.h"


class UTextBlock;
class AGS_PlayerState;

UCLASS()
class GANG_SQUIRREL_API UGS_LobbySlotWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable,Category="UI")
	void InitSlot(AGS_PlayerState* InPlayerState);
	
protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Text_NickName;
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UWidget> HostBadge;
	
private:
	UPROPERTY()
	TObjectPtr<AGS_PlayerState> BoundPlayerState;
	
	UFUNCTION()
	void OnNickNameChanged(const FString& NewName);
	
	void RefreshNickName();
	void RefreshHostBadge();
};
