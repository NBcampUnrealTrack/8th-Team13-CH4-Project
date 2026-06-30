
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

protected:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UTextBlock> TXT_Nickname;
};
