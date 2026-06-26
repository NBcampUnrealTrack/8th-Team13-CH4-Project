// Fill out your copyright notice in the Description page of Project Settings.


#include "GSPlayerController.h"

void AGSPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController() == false)
	{
		return;
	}

	FInputModeGameOnly IMGameOnly;
	SetInputMode(IMGameOnly);
}
