// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_StaminaBarWidget.generated.h"

class UProgressBar;

UCLASS()
class GANG_SQUIRREL_API UGS_StaminaBarWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetStaminaPercent(float InPercent);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> PB_Stamina;
};
