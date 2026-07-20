// Fill out your copyright notice in the Description page of Project Settings.


#include "GSFoodWidgetComponent.h"

#include "Gang_Squirrel/Food/GSFoodWidget.h"

UGSFoodWidgetComponent::UGSFoodWidgetComponent()
{
	
	static ConstructorHelpers::FClassFinder<UGSFoodWidget> FoodWidgetClass(TEXT("/Game/YJW/Food/FoodUI/WBP_FoodAnimeWidget"));
	if (FoodWidgetClass.Succeeded())
	{
		SetWidgetClass(FoodWidgetClass.Class);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Wrong class"));
	}
	
	
	SetWidgetSpace(EWidgetSpace::World);
	SetDrawAtDesiredSize(false);
	SetDrawSize(FVector2D(325.f, 325.f));
	SetRelativeScale3D(FVector(0.05));
	SetTwoSided(true);
	SetVisibility(false); 
    

	SetRelativeLocation(FVector(0.5f, 0.f, 20.f)); 
	SetRelativeRotation(FRotator(0.f, 0.f, 0.f)); 
	SetPivot(FVector2D(0.5f, 0.5f));
    
	PrimaryComponentTick.bCanEverTick = true;
}

void UGSFoodWidgetComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (!CachedPlayerController || !CachedCameraManager)
	{
		return;
	}
	
	FVector CameraLocation = CachedCameraManager->GetCameraLocation();

	FRotator Rotation =
		(CameraLocation - GetComponentLocation()).Rotation();

	Rotation.Pitch = 0.f;
	Rotation.Roll = 0.f;

	SetWorldRotation(Rotation);
}

void UGSFoodWidgetComponent::BeginPlay()
{
	Super::BeginPlay();
	
	CachedPlayerController  = GetWorld()->GetFirstPlayerController();
	
	if (CachedPlayerController)
	{
		CachedCameraManager = CachedPlayerController->PlayerCameraManager;
	}
}
