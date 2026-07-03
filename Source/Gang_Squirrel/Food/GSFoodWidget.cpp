// Fill out your copyright notice in the Description page of Project Settings.


#include "GSFoodWidget.h"

#include "Components/Image.h"
#include "Materials/MaterialInstanceDynamic.h"


void UGSFoodWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	UE_LOG(LogTemp, Warning, TEXT("FoodWidget NativeConstruct"));
	
	if (!ProgressImage)
	{
		UE_LOG(LogTemp, Error, TEXT("ProgressBar nullptr"));
		return;
	}
	
	UMaterialInterface* BaseMaterial = ProgressImage->GetBrush().GetResourceObject() ? Cast<UMaterialInterface>(ProgressImage->GetBrush().GetResourceObject()) : nullptr;
	
	if (MID)
	{
		MID->SetScalarParameterValue(TEXT("Percent"), 0.f);
		UE_LOG(LogTemp, Warning, TEXT("Successfully Created Dynamic Material Instance!!"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No Mid"));
	}
	
}

void UGSFoodWidget::SetProgress(float Value)
{
	if (!MID) return;
	
	MID->SetScalarParameterValue(TEXT("Percent"), Value);
}
