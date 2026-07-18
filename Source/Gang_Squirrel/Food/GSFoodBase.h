// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gang_Squirrel/SpawnSystem/GSSpawnPoint.h"
#include "GSFoodBase.generated.h"

class UGSFoodPrimaryDataAsset;
class USphereComponent;
class UGSFoodWidgetComponent;
class AGSCharacter;
class UGSFoodWidget;

UCLASS()
class GANG_SQUIRREL_API AGSFoodBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGSFoodBase();
	
	UPROPERTY(ReplicatedUsing=OnRep_Activate)
	bool bIsActive = false;
	
	UPROPERTY()
	bool bIsFilling = false;
	
	UPROPERTY(ReplicatedUsing=OnRep_FoodData)
	TObjectPtr<UGSFoodPrimaryDataAsset> FoodData;
	
	float CurrentEatenTime = 0.f;
	
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USphereComponent> SphereComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UGSFoodWidgetComponent> FoodWidgetComponent;
	
	UPROPERTY()
	TObjectPtr<AGSSpawnPoint> SpawnPoint;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> FoodWidgetClassDefaults;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Food | Visual", meta = (AllowPrivateAccess = "true"))
	class UMaterialInterface* OverlayMaterialAsset;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void Init(UGSFoodPrimaryDataAsset* InData);
	
	UFUNCTION()
	void OnRep_Activate();
	
	UFUNCTION()
	void OnRep_FoodData() const;
	
	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	void OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	void Deactivate();
	void Activate();
	
	void SetUIVisible(bool bShow);
	
	int32 Eaten();
	
	void StartWidgetAnime();
	
	void StopWidgetAnim();
	
	FORCEINLINE void  SetCurrentCharacter(AGSCharacter* InCharacter)  {CurrentCharacter = InCharacter;};
	
protected:
	
	FTimerHandle UIDelayTimerHandle;
	
private:
	
	UPROPERTY()
	AGSCharacter* CurrentCharacter = nullptr;
	
	bool bIsStartEating = true;
	
	UPROPERTY()
	TObjectPtr<UGSFoodWidget> CurrentFoodWidget;
	
	
};
