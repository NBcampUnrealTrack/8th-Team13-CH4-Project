// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GS_GameModeBase.generated.h"

class AGSSpawnManager;
/**
 * 
 */
UCLASS()
class GANG_SQUIRREL_API AGS_GameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AGSSpawnManager> SpawnManagerClass;
	
	void SpawnSpawnManager() const;
	
protected:
	
	virtual void BeginPlay() override;

};
