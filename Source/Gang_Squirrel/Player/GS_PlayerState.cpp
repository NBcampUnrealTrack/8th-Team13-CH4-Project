#include "GS_PlayerState.h"

#include "AbilitySystemComponent.h"


AGS_PlayerState::AGS_PlayerState()
{
	//PlayerState Network Settings
	bReplicates = true;
	// Create ASC
	AbilitySystemComp = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComp->SetIsReplicated(true);
	AbilitySystemComp->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
}

UAbilitySystemComponent* AGS_PlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComp;
}
