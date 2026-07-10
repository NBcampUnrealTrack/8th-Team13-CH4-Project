// Fill out your copyright notice in the Description page of Project Settings.


#include "GSHideoutPoint.h"

#include "Components/BoxComponent.h"
#include "Gang_Squirrel/Character/GSCharacter.h"


AGSHideoutPoint::AGSHideoutPoint()
{
	BoxComponent = CreateDefaultSubobject<UBoxComponent>("BoxComponent");
	RootComponent = BoxComponent;
	
	BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &AGSHideoutPoint::OnBeginOverlap);
	BoxComponent->OnComponentEndOverlap.AddDynamic(this, &AGSHideoutPoint::OnEndOverlap);
	
	
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AGSHideoutPoint::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGSHideoutPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AGSHideoutPoint::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AGSCharacter* CurrentCharacter = Cast<AGSCharacter>(OtherActor);
	
	if (CurrentCharacter)
	{
		CurrentCharacter->Server_NotifyAddScore(CurrentCharacter->GetTempScore());
		CurrentCharacter->ResetCheekSize();
	}
}

void AGSHideoutPoint::OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	
}

