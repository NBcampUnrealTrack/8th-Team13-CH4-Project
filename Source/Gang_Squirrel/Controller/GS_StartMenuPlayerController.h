// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GS_StartMenuPlayerController.generated.h"


UCLASS()
class GANG_SQUIRREL_API AGS_StartMenuPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> StartMenuWidgetClass;

public:
	UFUNCTION(BlueprintCallable)
	void RequestLobbyReady();

private:
	UFUNCTION(Server, Reliable)
	void ServerRequestLobbyReady();
};
