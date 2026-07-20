// Fill out your copyright notice in the Description page of Project Settings.


#include "GSFoodWidget.h"

#include "Materials/MaterialInstanceDynamic.h"
#include "Components/ProgressBar.h"


void UGSFoodWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
}

void UGSFoodWidget::SetProgress(float Value)
{
	if (!MID) return;
	
	MID->SetScalarParameterValue(TEXT("Percent"), Value);
	
	//UpdateWidget();
}

void UGSFoodWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	
}
