#include "GS_PlayerState.h"

#include "AbilitySystemComponent.h"
#include "Gang_Squirrel/GAS/AttributeSet/GS_PlayerAttributeSet.h"


AGS_PlayerState::AGS_PlayerState()
{
	//PlayerState Network Settings
	bReplicates = true;
	// Create ASC
	AbilitySystemComp = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComp->SetIsReplicated(true);
	AbilitySystemComp->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	
	// Create AttributeSet
	AttributeSet = CreateDefaultSubobject<UGS_PlayerAttributeSet>(TEXT("AttributeSet"));
}

UAbilitySystemComponent* AGS_PlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComp;
}
