#include "GA_AbilityBase.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Gang_Squirrel/GAS/Tags/GS_GamePlayTag.h"

UGA_AbilityBase::UGA_AbilityBase()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

bool UGA_AbilityBase::IsSameTeam(AActor* SourceActor, AActor* TargetActor)
{
	UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(SourceActor);
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	
	if (!SourceASC || !TargetASC)
	{
		return false;
	}
	
	const bool bBothPlayer = SourceASC->HasMatchingGameplayTag(TeamTag::TAG_Team_Player) && TargetASC->HasMatchingGameplayTag(TeamTag::TAG_Team_Player);
	const bool bBothEnemy = SourceASC->HasMatchingGameplayTag(TeamTag::TAG_Team_Enemy) && TargetASC->HasMatchingGameplayTag(TeamTag::TAG_Team_Enemy);
	
	return bBothPlayer || bBothEnemy;
}
