#include "GA_PlayerDeath.h"

#include "AbilitySystemComponent.h"
#include "EngineUtils.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerStart.h"
#include "Gang_Squirrel/Character/GSCharacter.h"
#include "Gang_Squirrel/GAS/Tags/GS_GamePlayTag.h"

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
			ASC->AddReplicatedLooseGameplayTag(StateTag::TAG_State_Dead);
		}
		
		// TempLogic
		for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
		{
			CachedRespawnLocation = It->GetActorLocation();
			CachedRespawnRotation = It->GetActorRotation();
			break;
		}
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
			GetWorld()->GetTimerManager().SetTimer(RespawnTimerHandle,this,&UGA_PlayerDeath::HandleRespawn,RespawnDelay,false);
		}
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

// Temp Method
void UGA_PlayerDeath::HandleRespawn()
{
	ACharacter* PlayerChar = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!PlayerChar)
	{
		return;
	}
	
	PlayerChar->SetActorLocation(CachedRespawnLocation);
	PlayerChar->SetActorRotation(CachedRespawnRotation);
	PlayerChar->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->RemoveReplicatedLooseGameplayTag(StateTag::TAG_State_Dead);
		
		if (GE_Respawn)
		{
			FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
			FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(GE_Respawn,1,ContextHandle);
			
			if (SpecHandle.IsValid())
			{
				ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}
	}
	
}
