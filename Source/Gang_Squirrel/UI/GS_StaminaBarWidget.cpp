// Fill out your copyright notice in the Description page of Project Settings.


#include "GS_StaminaBarWidget.h"
#include "Components/ProgressBar.h"

void UGS_StaminaBarWidget::SetStaminaPercent(float InPercent)
{
	if (PB_Stamina)
	{
		PB_Stamina->SetPercent(InPercent);
	}
}

