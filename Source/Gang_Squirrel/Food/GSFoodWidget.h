// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GSFoodWidget.generated.h"


class UImage;
/**
 * 
 */
UCLASS()
class GANG_SQUIRREL_API UGSFoodWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(meta = (BindWidget))
	UImage* ProgressImage;
	
	UFUNCTION(BlueprintCallable)
	void SetProgress(float Value);
	
	UPROPERTY()
	UMaterialInstanceDynamic* MID;
	
protected:
	
	virtual void NativeConstruct() override;
};
