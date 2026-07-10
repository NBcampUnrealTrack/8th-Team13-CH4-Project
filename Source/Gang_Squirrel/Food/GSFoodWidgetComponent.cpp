// Fill out your copyright notice in the Description page of Project Settings.


#include "GSFoodWidgetComponent.h"

#include "Gang_Squirrel/Food/GSFoodWidget.h"

UGSFoodWidgetComponent::UGSFoodWidgetComponent()
{
	
	static ConstructorHelpers::FClassFinder<UUserWidget> FoodWidgetClass(TEXT("/Game/YJW/Food/FoodUI/WBP_FoodWidget.WBP_FoodWidget_C"));
	if (FoodWidgetClass.Succeeded())
	{
		SetWidgetClass(FoodWidgetClass.Class);
	}
	
	
	SetWidgetSpace(EWidgetSpace::World);
	SetDrawAtDesiredSize(false);
	SetDrawSize(FVector2D(30.f, 30.f));
	SetTwoSided(true);
	SetVisibility(false); 
    

	SetRelativeLocation(FVector(0.5f, 0.f, 10.f)); 
	SetRelativeRotation(FRotator(0.f, 0.f, 0.f)); 
	SetPivot(FVector2D(0.5f, 0.5f));
    
	PrimaryComponentTick.bCanEverTick = true;
	
	PrimaryComponentTick.bCanEverTick = true;
}

void UGSFoodWidgetComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC)
	{
		return;
	}

	if (!PC->PlayerCameraManager)
	{
		return;
	}
	
	FVector CameraLocation = PC->PlayerCameraManager->GetCameraLocation();

	FRotator Rotation =
		(CameraLocation - GetComponentLocation()).Rotation();

	Rotation.Pitch = 0.f;
	Rotation.Roll = 0.f;

	SetWorldRotation(Rotation);
}

void UGSFoodWidgetComponent::BeginPlay()
{
	Super::BeginPlay();
}
