#include "GA_Sprint.h"

#include "AbilitySystemComponent.h"
#include "TimerManager.h"
#include "Gang_Squirrel/Character/GSCharacter.h"
#include "Gang_Squirrel/GAS/AttributeSet/GS_PlayerAttributeSet.h"
#include "Gang_Squirrel/GAS/Tags/GS_GamePlayTag.h"

UGA_Sprint::UGA_Sprint()
{
	AbilityTags.AddTag(AbilityTag::TAG_Ability_Sprint);
	ActivationBlockedTags.AddTag(StateTag::TAG_State_Dead);
}

void UGA_Sprint::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData
)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// UE_LOG(LogTemp, Warning, TEXT("[GA_Sprint] ActivateAbility called"));

	if (GetCurrentStamina() < MinStaminaToSprint)
	{
		// UE_LOG(LogTemp, Warning, TEXT("[GA_Sprint] Not enough stamina."));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (CommitAbility(Handle, ActorInfo, ActivationInfo) == false)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AGSCharacter* Character = Cast<AGSCharacter>(GetAvatarActorFromActorInfo());
	if (!Character)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ASC->AddLooseGameplayTag(StateTag::TAG_State_Sprinting);
	ASC->AddReplicatedLooseGameplayTag(StateTag::TAG_State_Sprinting);

	Character->StartSprintFromAbility();

	GetWorld()->GetTimerManager().ClearTimer(SprintCostTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(
		SprintCostTimerHandle,
		this,
		&UGA_Sprint::ApplySprintCost,
		StaminaCostInterval,
		true,
		0.f
	);
}

void UGA_Sprint::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled
)
{
	GetWorld()->GetTimerManager().ClearTimer(SprintCostTimerHandle);

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->RemoveLooseGameplayTag(StateTag::TAG_State_Sprinting);
		ASC->RemoveReplicatedLooseGameplayTag(StateTag::TAG_State_Sprinting);
	}

	if (AGSCharacter* Character = Cast<AGSCharacter>(GetAvatarActorFromActorInfo()))
	{
		Character->StopSprintFromAbility();
	}

	// UE_LOG(LogTemp, Warning, TEXT("[GA_Sprint] EndAbility called"));

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Sprint::ApplySprintCost()
{
	if (!CurrentActorInfo || !CurrentActorInfo->IsNetAuthority())
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		K2_EndAbility();
		return;
	}

	if (!GE_SprintStaminaCost)
	{
		
		K2_EndAbility();
		return;
	}

	const float CurrentStamina = GetCurrentStamina();

	if (CurrentStamina <= 0.f)
	{
		// UE_LOG(LogTemp, Warning, TEXT("[GA_Sprint] Stamina empty."));
		K2_EndAbility();
		return;
	}

	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	Context.AddSourceObject(this);

	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(GE_SprintStaminaCost, 1.f, Context);

	if (SpecHandle.IsValid())
	{
		ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

		/*UE_LOG(LogTemp, Warning, TEXT("[GA_Sprint] Stamina After Cost: %f"),
			ASC->GetNumericAttribute(UGS_PlayerAttributeSet::GetStaminaAttribute()));*/
	}

	if (GetCurrentStamina() <= 0.f)
	{
		K2_EndAbility();
	}
}

float UGA_Sprint::GetCurrentStamina() const
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		return 0.f;
	}

	return ASC->GetNumericAttribute(UGS_PlayerAttributeSet::GetStaminaAttribute());
}