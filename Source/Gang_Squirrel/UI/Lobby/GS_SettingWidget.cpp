// Fill out your copyright notice in the Description page of Project Settings.


#include "Gang_Squirrel/UI/Lobby/GS_SettingWidget.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Gang_Squirrel/EOS/GS_GameInstance.h"

void UGS_SettingWidget::NativeConstruct()
{
    Super::NativeConstruct();

    UGS_GameInstance* GSInst = GetGameInstance<UGS_GameInstance>();
    if (!GSInst) return;

    // 현재 값으로 초기화
    Slider_MouseSensitivity->SetValue(GSInst->MouseSensitivity);
    Text_MouseSensitivity->SetText(FText::AsNumber(FMath::RoundToInt(GSInst->MouseSensitivity)));

    Slider_Volume->SetValue(GSInst->MasterVolume);
    Text_Volume->SetText(FText::AsNumber(FMath::RoundToInt(GSInst->MasterVolume)));

    Slider_Brightness->SetValue(GSInst->ScreenBrightness);
    Text_Brightness->SetText(FText::AsNumber(FMath::RoundToInt(GSInst->ScreenBrightness)));

    Slider_MouseSensitivity->OnValueChanged.AddDynamic(this, &UGS_SettingWidget::OnMouseSensitivityChanged);
    Slider_Volume->OnValueChanged.AddDynamic(this, &UGS_SettingWidget::OnVolumeChanged);
    Slider_Brightness->OnValueChanged.AddDynamic(this, &UGS_SettingWidget::OnBrightnessChanged);
    close_Button->OnClicked.AddDynamic(this, &UGS_SettingWidget::OnCloseClicked);
}

void UGS_SettingWidget::OnMouseSensitivityChanged(float Value)
{
    if (UGS_GameInstance* GSInst = GetGameInstance<UGS_GameInstance>())
    {
        GSInst->SetMouseSensitivity(Value);
    }
    Text_MouseSensitivity->SetText(FText::AsNumber(FMath::RoundToInt(Value)));
}

void UGS_SettingWidget::OnVolumeChanged(float Value)
{
    if (UGS_GameInstance* GSInst = GetGameInstance<UGS_GameInstance>())
    {
        GSInst->SetMasterVolume(Value);
    }
    Text_Volume->SetText(FText::AsNumber(FMath::RoundToInt(Value)));
}

void UGS_SettingWidget::OnBrightnessChanged(float Value)
{
    if (UGS_GameInstance* GSInst = GetGameInstance<UGS_GameInstance>())
    {
        GSInst->SetScreenBrightness(Value);
    }
    Text_Brightness->SetText(FText::AsNumber(FMath::RoundToInt(Value)));
}

void UGS_SettingWidget::OnCloseClicked()
{
    RemoveFromParent();
}