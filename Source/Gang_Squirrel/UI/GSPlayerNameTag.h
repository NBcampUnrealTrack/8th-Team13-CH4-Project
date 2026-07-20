
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GSPlayerNameTag.generated.h"

UCLASS()
class GANG_SQUIRREL_API UGSPlayerNameTag : public UUserWidget
{
	GENERATED_BODY()
	
public:
    UFUNCTION(BlueprintCallable, Category = "UI")
    void SetNickname(const FString& NewName);
	
	UFUNCTION(BlueprintCallable,Category="UI")
	void SetReadyState(bool bReady);
protected:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UTextBlock> TXT_Nickname;
	
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<class UImage> Image_ReadyCheck;
};
