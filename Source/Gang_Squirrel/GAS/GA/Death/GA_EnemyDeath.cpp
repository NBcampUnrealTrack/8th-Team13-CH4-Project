#include "GA_EnemyDeath.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Gang_Squirrel/Enemy/GS_Enemy.h"
#include "Gang_Squirrel/GAS/Tags/GS_GamePlayTag.h"

UGA_EnemyDeath::UGA_EnemyDeath()
{
	AbilityTags.AddTag(AbilityTag::TAG_Ability_Death);
	ActivationBlockedTags.AddTag(StateTag::TAG_State_Dead);
}

void UGA_EnemyDeath::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	AGS_Enemy* Enemy = Cast<AGS_Enemy>(GetAvatarActorFromActorInfo());
	if (!Enemy)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	if (ActorInfo->IsNetAuthority())
	{
		Enemy->GetCharacterMovement()->DisableMovement();
		
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
		{
			ASC->AddReplicatedLooseGameplayTag(StateTag::TAG_State_Dead);
		}
	}
	
	if (AM_Death)
	{
		UAbilityTask_PlayMontageAndWait* TaskMontage = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this,NAME_None,AM_Death);
		TaskMontage->OnCompleted.AddDynamic(this,&UGA_EnemyDeath::K2_EndAbility);
		TaskMontage->OnInterrupted.AddDynamic(this,&UGA_EnemyDeath::K2_EndAbility);
		TaskMontage->OnCancelled.AddDynamic(this,&UGA_EnemyDeath::K2_EndAbility);
		
		TaskMontage->ReadyForActivation();
	}
	else
	{
		EndAbility(Handle,ActorInfo,ActivationInfo,true,false);
	}
}

void UGA_EnemyDeath::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (AGS_Enemy* Enemy = Cast<AGS_Enemy>(GetAvatarActorFromActorInfo()))
	{
		if (ActorInfo->IsNetAuthority())
		{
			Enemy->SetLifeSpan(2.f);
		}
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
