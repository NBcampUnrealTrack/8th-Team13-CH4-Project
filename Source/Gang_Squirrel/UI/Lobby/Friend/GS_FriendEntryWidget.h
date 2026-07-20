#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_FriendEntryWidget.generated.h"

class UButton;
class UTextBlock;
struct FGSFriendInfo;

UCLASS()
class GANG_SQUIRREL_API UGS_FriendEntryWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable,Category="UI")
	void InitEntry(const FGSFriendInfo& Info);
	UFUNCTION(BlueprintCallable,Category="UI")
	void OnInviteClicked();
	
protected:
	virtual void NativeConstruct() override;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Text_NickName;
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UButton> Button_Invite;
	
private:
	int32 FriendIndex = -1;
};
