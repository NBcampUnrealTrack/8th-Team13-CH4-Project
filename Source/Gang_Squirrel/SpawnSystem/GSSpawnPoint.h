// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GSSpawnPoint.generated.h"

class UBoxComponent;

UCLASS()
class GANG_SQUIRREL_API AGSSpawnPoint : public AActor
{
	GENERATED_BODY()
	
	UPROPERTY()
	int32 SpawnCount = 0;
	
public:	
	// Sets default values for this actor's properties
	AGSSpawnPoint();
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> SpawnArea;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpawnSetting")
	int32 MaxSpawnAmount = 1;
	
	UPROPERTY(EditAnywhere)
	int32 CurrentFoodCount = 0;
	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	FVector GetRandomLocation() const;

};
