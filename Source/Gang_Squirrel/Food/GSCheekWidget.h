// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GSCheekWidget.generated.h"

/**
 * 
 */
UCLASS()
class GANG_SQUIRREL_API UGSCheekWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	
	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetProgressValue(float NewPercent);
	
	UFUNCTION()
	void InitCheekWidget();
	
protected:
	
	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void OnProgressChanged(float NewPercent);
	
private:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
	float CurrentPercent = 0.0f;
	
	
};
