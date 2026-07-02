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

	UE_LOG(LogGAS, Warning, TEXT("[GA_PlayerDeath] ActivateAbility called - Player:%s, AM_Death:%s"), *GetNameSafe(GetAvatarActorFromActorInfo()), AM_Death ? TEXT("valid") : TEXT("NULL"));

	AGSCharacter* PlayerCharacter = Cast<AGSCharacter>(GetAvatarActorFromActorInfo());
	if (!PlayerCharacter)
	{
		UE_LOG(LogGAS, Error, TEXT("[GA_PlayerDeath] Cast to AGSCharacter FAILED"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UE_LOG(LogGAS, Warning, TEXT("[GA_PlayerDeath] IsNetAuthority:%s"), ActorInfo->IsNetAuthority() ? TEXT("true") : TEXT("false"));

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
		UE_LOG(LogGAS, Warning, TEXT("[GA_PlayerDeath] PlayerStart found:%s, CachedRespawnLocation:%s"), bFoundPlayerStart ? TEXT("true") : TEXT("false"), *CachedRespawnLocation.ToString());
	}
	
	if (AM_Death)
	{
		UAbilityTask_PlayMontageAndWait* TaskMontage = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this,NAME_None,AM_Death);
		TaskMontage->OnCompleted.AddDynamic(this,&UGA_PlayerDeath::K2_EndAbility);
		TaskMontage->OnInterrupted.AddDynamic(this,&UGA_PlayerDeath::K2_EndAbility);
		TaskMontage->OnCancelled.AddDynamic(this,&UGA_PlayerDeath::K2_EndAbility);
		TaskMontage->OnBlendOut.AddDynamic(this,&UGA_PlayerDeath::HandleDeathBlendOut);
		
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
	UE_LOG(LogGAS, Warning, TEXT("[GA_PlayerDeath] EndAbility called - Player:%s, IsNetAuthority:%s, bWasCancelled:%s"),
		*GetNameSafe(GetAvatarActorFromActorInfo()), ActorInfo->IsNetAuthority() ? TEXT("true") : TEXT("false"), bWasCancelled ? TEXT("true") : TEXT("false"));

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
	
	PlayerCharacter->NetMulticast_SetDeathPoseFrozen(false);
	PlayerChar->SetActorLocation(CachedRespawnLocation);
	PlayerChar->SetActorRotation(CachedRespawnRotation);
	PlayerChar->GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->RemoveLooseGameplayTag(StateTag::TAG_State_Dead);
		ASC->RemoveReplicatedLooseGameplayTag(StateTag::TAG_State_Dead);

		UE_LOG(LogGAS, Warning, TEXT("[GA_PlayerDeath] HandleRespawn - GE_Respawn:%s"), GE_Respawn ? *GE_Respawn->GetName() : TEXT("NULL"));

		if (GE_Respawn)
		{
			FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
			FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(GE_Respawn,1,ContextHandle);

			if (SpecHandle.IsValid())
			{
				FActiveGameplayEffectHandle AppliedHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
				UE_LOG(LogGAS, Warning, TEXT("[GA_PlayerDeath] HandleRespawn - GE_Respawn applied, WasSuccessfullyApplied:%s"), AppliedHandle.WasSuccessfullyApplied() ? TEXT("true") : TEXT("false"));
			}
		}
	}

}

void UGA_PlayerDeath::HandleDeathBlendOut()
{
	if (AGSCharacter* OwnerChar = Cast<AGSCharacter>(GetAvatarActorFromActorInfo()))
	{
		OwnerChar->NetMulticast_SetDeathPoseFrozen(true);
	}
	K2_EndAbility();
}
