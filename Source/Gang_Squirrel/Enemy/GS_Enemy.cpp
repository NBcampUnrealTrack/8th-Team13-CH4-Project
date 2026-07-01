#include "GS_Enemy.h"

#include "AbilitySystemComponent.h"
#include "Components/SphereComponent.h"
#include "Gang_Squirrel/GAS/GA/Attack/Enemy/GA_EnemyAttack.h"
#include "Gang_Squirrel/GAS/AttributeSet/GS_PlayerAttributeSet.h"


AGS_Enemy::AGS_Enemy()
{
	PrimaryActorTick.bCanEverTick = true;
	
#pragma region InitGASSystem
	//Create ASC
	EnemyAbilitySystemComp = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	EnemyAbilitySystemComp->SetIsReplicated(true);
	EnemyAbilitySystemComp->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	
	//Create AttributeSet
	EnemyAttributeSet = CreateDefaultSubobject<UGS_PlayerAttributeSet>(TEXT("AttributeSet"));
#pragma endregion 
	
#pragma region CombatComponent
	//Create CombatComponent
	SphereComp_LeftHand = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent_LeftHand"));
	SphereComp_LeftHand->SetupAttachment(GetMesh(), FName("L_Hand"));
	
	SphereComp_RightHand = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent_RightHand"));
	SphereComp_RightHand->SetupAttachment(GetMesh(), FName("R_Hand"));
#pragma endregion
}

void AGS_Enemy::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		EnemyAbilitySystemComp->InitAbilityActorInfo(this,this);
		EnemyAbilitySystemComp->GiveAbility(FGameplayAbilitySpec(GA_Attack,1));
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

USphereComponent* AGS_Enemy::GetCombatCollision(EHandCombatType HandType) const
{
	USphereComponent* CombatHandComp = nullptr;
	
	if (HandType == EHandCombatType::RightCombatHand)
	{
		CombatHandComp = SphereComp_RightHand;
	}
	else
	{
		CombatHandComp = SphereComp_LeftHand;
	}
	
	return CombatHandComp;
}