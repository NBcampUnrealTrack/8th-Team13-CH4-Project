#include "GA_EnemyDeath.h"

#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Gang_Squirrel/Enemy/GS_Enemy.h"
#include "Gang_Squirrel/GAS/Tags/GS_GamePlayTag.h"
#include "Gang_Squirrel/Gang_Squirrel.h"
#include "Gang_Squirrel/Game/GS_GameModeBase.h"
#include "Gang_Squirrel/Player/GS_PlayerState.h"

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
		Enemy->SetRotationTarget(nullptr,0.f);
		
		if (AAIController* AIController = Cast<AAIController>(Enemy->GetController()))
		{
			if (UBrainComponent* BrainComp = AIController->GetBrainComponent())
			{
				BrainComp->StopLogic(TEXT("Enemy Died"));
			}
		}

		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
		{
			ASC->AddLooseGameplayTag(StateTag::TAG_State_Dead);
			ASC->AddReplicatedLooseGameplayTag(StateTag::TAG_State_Dead);

			ASC->CancelAllAbilities(this);
		}
		
		Enemy->NetMulticast_PlayDeathSound();
		
		Enemy->NetMulticast_SetFullRagdollEnable(true);
		Enemy->NetMulticast_ApplyRagdollImpulse(Enemy->GetLastHitImpulseDirection() * DeathImpulseStrength, Enemy->GetRagdollStartBone());
	}
	
	EndAbility(Handle,ActorInfo,ActivationInfo,true,false);
}

void UGA_EnemyDeath::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (AGS_Enemy* Enemy = Cast<AGS_Enemy>(GetAvatarActorFromActorInfo()))
	{
		if (ActorInfo->IsNetAuthority())
		{
			Enemy->ScheduleReturnToPool(6.f);
		}

		if (AGS_PlayerState* KillerPS = Enemy->GetKillerPlayerState())
		{
			KillerPS->AddKillCount();   // 추가: 서버에서 KillCount 증가

			if (AGS_GameModeBase* GM = Cast<AGS_GameModeBase>(Enemy->GetWorld()->GetAuthGameMode()))
			{
				GM->GiveRandomReward(KillerPS);
			}
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

