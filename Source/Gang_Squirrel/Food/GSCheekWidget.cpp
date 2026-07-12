// Fill out your copyright notice in the Description page of Project Settings.


#include "GSCheekWidget.h"

#include "Components/TextBlock.h"

void UGSCheekWidget::SetProgressValue(float NewPercent)
{
	CurrentPercent = NewPercent;
	
	OnProgressChanged(CurrentPercent);
	
	if (TextBlock)
	{
		int32 PercentInt = FMath::TruncToInt(NewPercent * 100.0f);
		
		TextBlock->SetText(
	FText::FromString(FString::Printf(TEXT("%d%%"), PercentInt))
);
	}
}

void UGSCheekWidget::InitCheekWidget()
{
	CurrentPercent = 0.0f;
	
	OnProgressChanged(CurrentPercent);
	// UE_LOG(LogTemp, Warning, TEXT("UGSCheekWidget::InitCheekWidget()"));
}
