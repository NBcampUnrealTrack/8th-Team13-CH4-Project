// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GSSlideWidget.generated.h"

/**
 * 
 */
UCLASS()
class GANG_SQUIRREL_API UGSSlideWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateSlideWidget(int32 ScoreAmount);
	
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateText(int32 Number);
};
