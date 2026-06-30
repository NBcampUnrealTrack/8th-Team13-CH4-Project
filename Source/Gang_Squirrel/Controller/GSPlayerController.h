// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GSPlayerController.generated.h"

UCLASS()
class GANG_SQUIRREL_API AGSPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	
	//Call after entring nickname
	UFUNCTION(BlueprintCallable, Category = "Player")
	void SubmitNickname(const FString& Nickname);

private:
	UFUNCTION(Server, Reliable)
	void ServerSetNickname(const FString& Nickname);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> NicknameInputWidgetClass;
};


