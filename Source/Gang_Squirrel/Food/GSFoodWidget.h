// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GSFoodWidget.generated.h"


class UProgressBar;
/**
 * 
 */
UCLASS()
class GANG_SQUIRREL_API UGSFoodWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	
	UFUNCTION(BlueprintCallable)
	void SetProgress(float Value);
	
	UPROPERTY()
	UMaterialInstanceDynamic* MID;
	
	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void UpdateWidget();
	
	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void StopWidget();
	
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
protected:
	
	virtual void NativeConstruct() override;
};
