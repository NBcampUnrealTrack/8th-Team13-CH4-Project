// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GSHideoutPoint.generated.h"

class UBoxComponent;
UCLASS()
class GANG_SQUIRREL_API AGSHideoutPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGSHideoutPoint();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hideout")
	TObjectPtr<UBoxComponent> BoxComponent;

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	void OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// For Mapping Slot Index(In PlayerState)
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Hideout")
	int32 SlotIndex = 0;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
