// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_NicknameInputWidget.generated.h"

/**
 * 
 */
UCLASS()
class GANG_SQUIRREL_API UGS_NicknameInputWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
    void ShowError(const FString& ErrorMessage);
    void ClearError();

protected:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UTextBlock> TXT_Error;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UEditableTextBox> NicknameInput_Text;

public:
    UFUNCTION(BlueprintCallable)
    FString GetNicknameText() const;
};
