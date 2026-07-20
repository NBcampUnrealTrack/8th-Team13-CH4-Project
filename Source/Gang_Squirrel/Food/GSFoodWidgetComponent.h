// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "GSFoodWidgetComponent.generated.h"

class UGSFoodWidget;
/**
 * 
 */
UCLASS(ClassGroup=(UI), meta=(BlueprintSpawnableComponent))
class GANG_SQUIRREL_API UGSFoodWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()
	
public:
	
	UGSFoodWidgetComponent();
	
protected:
	
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	virtual void BeginPlay() override;
	
private:
	
	UPROPERTY()
	TObjectPtr<APlayerController> CachedPlayerController;
	
	UPROPERTY()
	TObjectPtr<APlayerCameraManager> CachedCameraManager;
};
