#include "GS_Enemy.h"

#include "AbilitySystemComponent.h"
#include "Gang_Squirrel/GAS/AttributeSet/GS_PlayerAttributeSet.h"


AGS_Enemy::AGS_Enemy()
{
	PrimaryActorTick.bCanEverTick = true;
	
	//Create ASC
	EnemyAbilitySystemComp = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	EnemyAbilitySystemComp->SetIsReplicated(true);
	EnemyAbilitySystemComp->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	
	//Create AttributeSet
	EnemyAttributeSet = CreateDefaultSubobject<UGS_PlayerAttributeSet>(TEXT("AttributeSet"));
}


void AGS_Enemy::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		EnemyAbilitySystemComp->InitAbilityActorInfo(this,this);
	}
}


void AGS_Enemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

UAbilitySystemComponent* AGS_Enemy::GetAbilitySystemComponent() const
{
	return EnemyAbilitySystemComp;
}

