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
	
	UE_LOG(LogGAS, Warning, TEXT("[GA_EnemyDeath] ActivateAbility called - Enemy:%s"), *GetNameSafe(GetAvatarActorFromActorInfo()));
	
	AGS_Enemy* Enemy = Cast<AGS_Enemy>(GetAvatarActorFromActorInfo());
	if (!Enemy)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	UE_LOG(LogGAS, Warning, TEXT("[GA_EnemyDeath] IsNetAuthority:%s"), ActorInfo->IsNetAuthority() ? TEXT("true") : TEXT("false"));

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
			UE_LOG(LogGAS, Warning, TEXT("[GA_EnemyDeath] Added State.Dead tag, HasTag immediately after:%s"), ASC->HasMatchingGameplayTag(StateTag::TAG_State_Dead) ? TEXT("true") : TEXT("false"));
		}
		else
		{
			UE_LOG(LogGAS, Error, TEXT("[GA_EnemyDeath] ASC from ActorInfo is NULL"));
		}
	}
	
	if (AM_Death)
	{
		UAbilityTask_PlayMontageAndWait* TaskMontage = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this,NAME_None,AM_Death);
		TaskMontage->OnCompleted.AddDynamic(this,&UGA_EnemyDeath::K2_EndAbility);
		TaskMontage->OnInterrupted.AddDynamic(this,&UGA_EnemyDeath::K2_EndAbility);
		TaskMontage->OnCancelled.AddDynamic(this,&UGA_EnemyDeath::K2_EndAbility);
		TaskMontage->OnBlendOut.AddDynamic(this,&UGA_EnemyDeath::HandleDeathBlendOut);
		
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
	UE_LOG(LogGAS, Warning, TEXT("[GA_EnemyDeath] EndAbility called - Enemy:%s, IsNetAuthority:%s, bWasCancelled:%s"),
		*GetNameSafe(GetAvatarActorFromActorInfo()), ActorInfo->IsNetAuthority() ? TEXT("true") : TEXT("false"), bWasCancelled ? TEXT("true") : TEXT("false"));

	if (AGS_Enemy* Enemy = Cast<AGS_Enemy>(GetAvatarActorFromActorInfo()))
	{
		if (ActorInfo->IsNetAuthority())
		{
			Enemy->SetLifeSpan(2.f);
			UE_LOG(LogGAS, Warning, TEXT("[GA_EnemyDeath] SetLifeSpan(2.f) called on %s"), *GetNameSafe(Enemy));
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

void UGA_EnemyDeath::HandleDeathBlendOut()
{
	if (AGS_Enemy* Enemy = Cast<AGS_Enemy>(GetAvatarActorFromActorInfo()))
	{
		Enemy->NetMultiCast_FreezeDeathPose();
	}
	K2_EndAbility();
}
