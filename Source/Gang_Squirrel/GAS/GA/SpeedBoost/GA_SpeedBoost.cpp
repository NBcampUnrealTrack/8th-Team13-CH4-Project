// Fill out your copyright notice in the Description page of Project Settings.


#include "Gang_Squirrel/GAS/GA/SpeedBoost/GA_SpeedBoost.h"
#include "AbilitySystemComponent.h"
#include "Gang_Squirrel/Character/GSCharacter.h"
#include "Gang_Squirrel/GAS/Tags/GS_GamePlayTag.h"

UGA_SpeedBoost::UGA_SpeedBoost()
{
	AbilityTags.AddTag(AbilityTag::TAG_Ability_SpeedBoost);
	ActivationBlockedTags.AddTag(StateTag::TAG_State_Dead);
}

void UGA_SpeedBoost::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UE_LOG(LogTemp, Warning, TEXT("[GA_SpeedBoost] ActivateAbility called, IsNetAuthority: %s"),
		ActorInfo->IsNetAuthority() ? TEXT("true") : TEXT("false"));

	AGSCharacter* Character = Cast<AGSCharacter>(GetAvatarActorFromActorInfo());
	if (!Character)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->AddLooseGameplayTag(StatusTag::TAG_Status_SpeedBoost);
		ASC->AddReplicatedLooseGameplayTag(StatusTag::TAG_Status_SpeedBoost);
	}
	Character->StartSpeedBoostFromAbility(SpeedMultiplier);

	GetWorld()->GetTimerManager().SetTimer(
		SpeedBoostTimerHandle,
		this,
		&UGA_SpeedBoost::HandleDurationExpired,
		Duration,
		false
	);

}

void UGA_SpeedBoost::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	GetWorld()->GetTimerManager().ClearTimer(SpeedBoostTimerHandle);

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->RemoveLooseGameplayTag(StatusTag::TAG_Status_SpeedBoost);
		ASC->RemoveReplicatedLooseGameplayTag(StatusTag::TAG_Status_SpeedBoost);
	}

	if (AGSCharacter* Character = Cast<AGSCharacter>(GetAvatarActorFromActorInfo()))
	{
		Character->StopSpeedBoostFromAbility();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_SpeedBoost::HandleDurationExpired()
{
	K2_EndAbility();
}
