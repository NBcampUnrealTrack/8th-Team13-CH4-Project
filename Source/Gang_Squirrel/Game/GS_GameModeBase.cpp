// Fill out your copyright notice in the Description page of Project Settings.


#include "GS_GameModeBase.h"

#include "Gang_Squirrel/SpawnSystem/GSSpawnManager.h"
#include "Gang_Squirrel/SpawnSystem/GSSpawnManager.h"

void AGS_GameModeBase::BeginPlay()
{
	Super::BeginPlay();
	
	SpawnSpawnManager();
}

void AGS_GameModeBase::SpawnSpawnManager() const
{
	if (!IsValid(SpawnManagerClass)) return;
	AGSSpawnManager* SpawnManager = GetWorld()->SpawnActor<AGSSpawnManager>(SpawnManagerClass);
}
