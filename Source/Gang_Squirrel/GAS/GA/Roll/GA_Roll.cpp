#include "GA_Roll.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Gang_Squirrel/GAS/AttributeSet/GS_PlayerAttributeSet.h"
#include "Gang_Squirrel/GAS/Tags/GS_GamePlayTag.h"
#include "Gang_Squirrel/Character/GSCharacter.h"

UGA_Roll::UGA_Roll()
{
	AbilityTags.AddTag(AbilityTag::TAG_Ability_Roll);
	ActivationBlockedTags.AddTag(StateTag::TAG_State_Dead);
}

void UGA_Roll::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData
)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UE_LOG(LogTemp, Warning, TEXT("[GA_Roll] ActivateAbility called"));

	const float CurrentStamina = GetCurrentStamina();

	UE_LOG(LogTemp, Warning, TEXT("[GA_Roll] CurrentStamina: %f / Cost: %f"), CurrentStamina, RollStaminaCost);

	if (GetCurrentStamina() < RollStaminaCost)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GA_Roll] No Stamina"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (CommitAbility(Handle, ActorInfo, ActivationInfo) == false)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ASC == nullptr)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (GE_RollStamina)
	{
		FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
		Context.AddSourceObject(this);

		FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(GE_RollStamina, 1.f, Context);

		if (SpecHandle.IsValid())
		{
			ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

			UE_LOG(LogTemp, Warning, TEXT("[GA_Roll] Stamina After Cost: %f"),
				ASC->GetNumericAttribute(UGS_PlayerAttributeSet::GetStaminaAttribute()));
		
			if (AGSCharacter* Character = Cast<AGSCharacter>(GetAvatarActorFromActorInfo()))
			{
				Character->RollFromAbility();
			}

			EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
			return;
		}
	}

	/*if (AM_Roll)
	{
		UAbilityTask_PlayMontageAndWait* TaskMontage =
			UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, AM_Roll);

		TaskMontage->OnCompleted.AddDynamic(this, &UGA_Roll::K2_EndAbility);
		TaskMontage->OnCancelled.AddDynamic(this, &UGA_Roll::K2_EndAbility);
		TaskMontage->OnInterrupted.AddDynamic(this, &UGA_Roll::K2_EndAbility);

		TaskMontage->ReadyForActivation();
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}*/
}

float UGA_Roll::GetCurrentStamina() const
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ASC == nullptr)
	{
		return 0.f;
	}

	return ASC->GetNumericAttribute(UGS_PlayerAttributeSet::GetStaminaAttribute());
}
