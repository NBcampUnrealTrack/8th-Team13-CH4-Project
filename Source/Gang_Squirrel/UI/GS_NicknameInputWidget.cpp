// Fill out your copyright notice in the Description page of Project Settings.


#include "Gang_Squirrel/UI/GS_NicknameInputWidget.h"
#include "Components/TextBlock.h"
#include "Components/EditableTextBox.h"

void UGS_NicknameInputWidget::ShowError(const FString& ErrorMessage)
{
    if (IsValid(TXT_Error))
    {
        TXT_Error->SetText(FText::FromString(ErrorMessage));
        TXT_Error->SetVisibility(ESlateVisibility::Visible);
    }
}

void UGS_NicknameInputWidget::ClearError()
{
    if (IsValid(TXT_Error))
    {
        TXT_Error->SetVisibility(ESlateVisibility::Hidden);
    }
}

FString UGS_NicknameInputWidget::GetNicknameText() const
{
    if (IsValid(NicknameInput_Text))
    {
        return NicknameInput_Text->GetText().ToString();
    }
    return FString();
}
