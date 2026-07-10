// Fill out your copyright notice in the Description page of Project Settings.


#include "GSCheekWidget.h"

void UGSCheekWidget::SetProgressValue(float NewPercent)
{
	CurrentPercent = NewPercent;
	
	OnProgressChanged(CurrentPercent);
}

void UGSCheekWidget::InitCheekWidget()
{
	CurrentPercent = 0.0f;
	
	OnProgressChanged(CurrentPercent);
	UE_LOG(LogTemp, Warning, TEXT("UGSCheekWidget::InitCheekWidget()"));
}
