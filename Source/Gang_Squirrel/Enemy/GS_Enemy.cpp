#include "GS_Enemy.h"



AGS_Enemy::AGS_Enemy()
{
	
	PrimaryActorTick.bCanEverTick = true;
}


void AGS_Enemy::BeginPlay()
{
	Super::BeginPlay();
	
}


void AGS_Enemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


void AGS_Enemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

