// Fill out your copyright notice in the Description page of Project Settings.


#include "GS_StaminaBarWidget.h"
#include "Components/ProgressBar.h"


void UGS_StaminaBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	TargetStaminaPercent = 1.f;
	CurrentDisplayPercent = 1.f;

	if (PB_Stamina)
	{
		PB_Stamina->SetPercent(CurrentDisplayPercent);
	}
}

void UGS_StaminaBarWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	CurrentDisplayPercent = FMath::FInterpTo(
		CurrentDisplayPercent,
		TargetStaminaPercent,
		InDeltaTime,
		StaminaInterpSpeed
	);

	if (PB_Stamina)
	{
		PB_Stamina->SetPercent(CurrentDisplayPercent);
	}
}

void UGS_StaminaBarWidget::SetStaminaPercent(float InPercent)
{
	TargetStaminaPercent = FMath::Clamp(InPercent, 0.f, 1.f);
}
