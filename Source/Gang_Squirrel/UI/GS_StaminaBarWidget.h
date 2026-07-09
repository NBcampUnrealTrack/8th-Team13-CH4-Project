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
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> PB_Stamina;

private:
	float TargetStaminaPercent = 1.f;
	float CurrentDisplayPercent = 1.f;

	UPROPERTY(EditAnywhere, Category = "Stamina")
	float StaminaInterpSpeed = 4.f;
};
