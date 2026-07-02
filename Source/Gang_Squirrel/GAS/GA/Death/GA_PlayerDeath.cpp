#include "GA_PlayerDeath.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Gang_Squirrel/Character/GSCharacter.h"
#include "Gang_Squirrel/GAS/Tags/GS_GamePlayTag.h"

UGA_PlayerDeath::UGA_PlayerDeath()
{
	AbilityTags.AddTag(AbilityTag::TAG_Ability_Death);
}

void UGA_PlayerDeath::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	AGSCharacter* PlayerCharacter = Cast<AGSCharacter>(GetAvatarActorFromActorInfo());
	if (!PlayerCharacter)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	if (ActorInfo->IsNetAuthority())
	{
		PlayerCharacter->GetCharacterMovement()->DisableMovement();
		PlayerCharacter->SetActorEnableCollision(false);
	}
	
	if (AM_Death)
	{
		UAbilityTask_PlayMontageAndWait* TaskMontage = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this,NAME_None,AM_Death);
		TaskMontage->OnCompleted.AddDynamic(this,&UGA_PlayerDeath::K2_EndAbility);
		TaskMontage->OnInterrupted.AddDynamic(this,&UGA_PlayerDeath::K2_EndAbility);
		TaskMontage->OnCancelled.AddDynamic(this,&UGA_PlayerDeath::K2_EndAbility);
		
		TaskMontage->ReadyForActivation();
	}
	else
	{
		EndAbility(Handle,ActorInfo,ActivationInfo,true,false);
	}
}

void UGA_PlayerDeath::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (AGSCharacter* PlayerCharacter = Cast<AGSCharacter>(GetAvatarActorFromActorInfo()))
	{
		if (ActorInfo->IsNetAuthority())
		{
			
		}
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
