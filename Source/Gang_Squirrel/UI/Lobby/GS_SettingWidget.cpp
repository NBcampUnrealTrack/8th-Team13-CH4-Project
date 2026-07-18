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
    // RemoveFromParent()로 완전히 제거하면 다음에 다시 열 때 AddToViewport가 재호출되면서
    // NativeConstruct가 다시 실행되어 델리게이트가 중복 바인딩되어 크래시가 난다.
    // GameInstance::ToggleSettingsWidget과 동일하게 Visibility만 바꿔서 위젯을 유지한다.
    SetVisibility(ESlateVisibility::Collapsed);
}