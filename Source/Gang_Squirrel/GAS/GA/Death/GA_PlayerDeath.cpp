#include "GA_PlayerDeath.h"

#include "AbilitySystemComponent.h"
#include "EngineUtils.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerStart.h"
#include "Gang_Squirrel/Character/GSCharacter.h"
#include "Gang_Squirrel/GAS/Tags/GS_GamePlayTag.h"
#include "Gang_Squirrel/Gang_Squirrel.h"

UGA_PlayerDeath::UGA_PlayerDeath()
{
	AbilityTags.AddTag(AbilityTag::TAG_Ability_Death);
	ActivationBlockedTags.AddTag(StateTag::TAG_State_Dead);
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

		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
		{
			ASC->AddLooseGameplayTag(StateTag::TAG_State_Dead);
			ASC->AddReplicatedLooseGameplayTag(StateTag::TAG_State_Dead);
		}

		// TempLogic
		bool bFoundPlayerStart = false;
		for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
		{
			CachedRespawnLocation = It->GetActorLocation();
			CachedRespawnRotation = It->GetActorRotation();
			bFoundPlayerStart = true;
			break;
		}
		
		PlayerCharacter->NetMulticast_SetFullRagdollEnable(true);
		PlayerCharacter->NetMulticast_ApplyRagdollImpulse(PlayerCharacter->GetLastHitImpulseDirection() * DeathImpulseStrength, PlayerCharacter->GetRagdollStartBone());
	}
	
	EndAbility(Handle,ActorInfo,ActivationInfo,true,false);
}

void UGA_PlayerDeath::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	
	if (AGSCharacter* PlayerCharacter = Cast<AGSCharacter>(GetAvatarActorFromActorInfo()))
	{
		if (ActorInfo->IsNetAuthority())
		{
			TWeakObjectPtr<UGA_PlayerDeath> WeakThis(this);
			GetWorld()->GetTimerManager().SetTimer(RespawnTimerHandle,[WeakThis]()
			{
				if (WeakThis.IsValid())
				{
					WeakThis->HandleRespawn();
				}
			},RespawnDelay,false);
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

// Temp Method
void UGA_PlayerDeath::HandleRespawn()
{
	UE_LOG(LogGAS, Warning, TEXT("[GA_PlayerDeath] HandleRespawn called - Player:%s, CachedRespawnLocation:%s"), *GetNameSafe(GetAvatarActorFromActorInfo()), *CachedRespawnLocation.ToString());

	ACharacter* PlayerChar = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!PlayerChar)
	{
		UE_LOG(LogGAS, Error, TEXT("[GA_PlayerDeath] HandleRespawn - Cast to ACharacter FAILED"));
		return;
	}

	AGSCharacter* PlayerCharacter = Cast<AGSCharacter>(PlayerChar);
	if (!PlayerCharacter)
	{
		return;
	}
	
	PlayerCharacter->NetMulticast_SetFullRagdollEnable(false);
	PlayerChar->SetActorLocation(CachedRespawnLocation);
	PlayerChar->SetActorRotation(CachedRespawnRotation);
	PlayerChar->GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->RemoveLooseGameplayTag(StateTag::TAG_State_Dead);
		ASC->RemoveReplicatedLooseGameplayTag(StateTag::TAG_State_Dead);
		
		if (GE_Respawn)
		{
			FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
			FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(GE_Respawn,1,ContextHandle);

			if (SpecHandle.IsValid())
			{
				FActiveGameplayEffectHandle AppliedHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
				
			}
		}
	}
}