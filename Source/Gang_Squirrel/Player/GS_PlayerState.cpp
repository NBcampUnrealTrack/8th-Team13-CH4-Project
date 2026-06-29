#include "GS_PlayerState.h"

#include "AbilitySystemComponent.h"
#include "Gang_Squirrel/GAS/AttributeSet/GS_PlayerAttributeSet.h"
#include "Net/UnrealNetwork.h"


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

void AGS_PlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGS_PlayerState, PlayerNickname);
}

void AGS_PlayerState::SetPlayerNickname(const FString& NewName)
{
	PlayerNickname = NewName;
	OnPlayerNameChanged.Broadcast(PlayerNickname);
}

void AGS_PlayerState::OnRep_PlayerNickname()
{
	OnPlayerNameChanged.Broadcast(PlayerNickname);
}
