#include "GA_DropKick.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Gang_Squirrel/Character/GSCharacter.h"
#include "Gang_Squirrel/GAS/Tags/GS_GamePlayTag.h"

UGA_DropKick::UGA_DropKick()
{
	AbilityTags.AddTag(AbilityTag::TAG_Ability_DropKick);
	ActivationBlockedTags.AddTag(StateTag::TAG_State_Dead);

	// DropKick has no combo state to preserve (unlike GA_Attack), so it's safe to let GAS
	// force-end a stuck-active instance and retrigger cleanly instead of silently rejecting reactivation.
	bRetriggerInstancedAbility = true;
}

void UGA_DropKick::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AGSCharacter* PlayerCharacter = Cast<AGSCharacter>(GetAvatarActorFromActorInfo());
	
	if (!PlayerCharacter || !AM_DropKick)
	{
		EndAbility(Handle,ActorInfo,ActivationInfo,true,true);
		return;
	}
	
	HitActors.Empty();
	
	if (ActorInfo->IsNetAuthority())
	{
		GetAbilitySystemComponentFromActorInfo()->AbilityTargetDataSetDelegate(Handle,ActivationInfo.GetActivationPredictionKey()).AddUObject(this,&UGA_DropKick::OnTargetReceived);
	}
	
	CurrentMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None,AM_DropKick,1.f);
	CurrentMontageTask->OnCompleted.AddDynamic(this, &UGA_DropKick::K2_EndAbility);
	CurrentMontageTask->OnInterrupted.AddDynamic(this, &UGA_DropKick::K2_EndAbility);
	CurrentMontageTask->OnCancelled.AddDynamic(this, &UGA_DropKick::K2_EndAbility);
	CurrentMontageTask->ReadyForActivation();
}

void UGA_DropKick::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (ActorInfo->IsNetAuthority())
	{
		GetAbilitySystemComponentFromActorInfo()->AbilityTargetDataSetDelegate(Handle,ActivationInfo.GetActivationPredictionKey()).RemoveAll(this);
	}
	
	HitActors.Empty();
	CurrentMontageTask = nullptr;
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_DropKick::OnAttackTraceHit(AActor* HitActor, const FHitResult& Hit)
{
	if (!HitActor || HitActor == GetAvatarActorFromActorInfo() || HitActors.Contains(HitActor))
	{
		return;
	}
	
	HitActors.Add(HitActor);
	
	FGameplayAbilityTargetDataHandle TargetDataHandle;
	FGameplayAbilityTargetData_SingleTargetHit* TargetData = new FGameplayAbilityTargetData_SingleTargetHit(Hit);
	TargetDataHandle.Add(TargetData);
	
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		return;
	}
	
	ASC->CallServerSetReplicatedTargetData(GetCurrentAbilitySpecHandle(), GetCurrentActivationInfo().GetActivationPredictionKey(), TargetDataHandle,FGameplayTag::EmptyTag, ASC->ScopedPredictionKey);
}

void UGA_DropKick::OnTargetReceived(const FGameplayAbilityTargetDataHandle& TargetData, FGameplayTag ActivationTag)
{
	GetAbilitySystemComponentFromActorInfo()->ConsumeClientReplicatedTargetData(GetCurrentAbilitySpecHandle(),GetCurrentActivationInfo().GetActivationPredictionKey());
	
	if (!GE_Damage)
	{
		return;
	}
	
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	
	FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
	Context.AddSourceObject(this);
	
	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(GE_Damage, 1.f, Context);
	if (!SpecHandle.IsValid())
	{
		return;
	}
	
	for (const TSharedPtr<FGameplayAbilityTargetData>& Data : TargetData.Data)
	{
		if (!Data.IsValid())
		{
			continue;
		}
		
		const FHitResult* HitResultPtr = Data->GetHitResult();
		
		for (TWeakObjectPtr<AActor> TargetActor : Data->GetActors())
		{
			if (!TargetActor.IsValid())
			{
				continue;
			}
			
			UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor.Get());
			
			if (TargetASC)
			{
				if (IsSameTeam(GetAvatarActorFromActorInfo(), TargetActor.Get()))
				{
					continue;
				}
				SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
				
				if (IGS_RagdollReactorInterface* TargetReactor = Cast<IGS_RagdollReactorInterface>(TargetActor.Get()))
				{
					const FName HitBone = (HitResultPtr && HitResultPtr->BoneName != NAME_None) ? HitResultPtr->BoneName : TargetReactor->GetRagdollStartBone();
					const FVector ImpulseDir = (HitResultPtr && !HitResultPtr->ImpactNormal.IsNearlyZero()) ? -HitResultPtr->ImpactNormal : GetAvatarActorFromActorInfo()->GetActorForwardVector();
					
					TargetReactor->Applyknockdown(ImpulseDir * KnockdownImpulseStrength, HitBone, KnockdownDuration);
				}
			}
		}
	}
}
