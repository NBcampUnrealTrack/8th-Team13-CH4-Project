// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_SettingWidget.generated.h"


class USlider;
class UTextBlock;
class UButton;

/**
 * 
 */
UCLASS()
class GANG_SQUIRREL_API UGS_SettingWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
    virtual void NativeConstruct() override;
    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

protected:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<USlider> Slider_MouseSensitivity;
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> Text_MouseSensitivity;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<USlider> Slider_Volume;
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> Text_Volume;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<USlider> Slider_Brightness;
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> Text_Brightness;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> close_Button;

private:
    UFUNCTION()
    void OnMouseSensitivityChanged(float Value);
    UFUNCTION()
    void OnVolumeChanged(float Value);
    UFUNCTION()
    void OnBrightnessChanged(float Value);
    UFUNCTION()
    void OnCloseClicked();
    
    void CloseSettings();
};
