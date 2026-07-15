#include "GA_Attack.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Components/SphereComponent.h"
#include "Gang_Squirrel/Character/GSCharacter.h"
#include "Gang_Squirrel/Gang_Squirrel.h"
#include "Gang_Squirrel/GAS/Tags/GS_GamePlayTag.h"

UGA_Attack::UGA_Attack()
{
	AbilityTags.AddTag(AbilityTag::TAG_Ability_Attack);
	ActivationBlockedTags.AddTag(StateTag::TAG_State_Dead);
}

void UGA_Attack::RequestComboInput()
{
	if (!CurrentMontageTask || !bComboWindowOpen)
	{
		return;
	}
	
	bIsSecondCombo = !bIsSecondCombo;
	bComboWindowOpen = false;
	HitActors.Empty();
	
	const FName NextSection = bIsSecondCombo ? ComboSection_Second : ComboSection_First;
	GetAbilitySystemComponentFromActorInfo()->CurrentMontageJumpToSection(NextSection);
}

void UGA_Attack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                 const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	AGSCharacter* Character = Cast<AGSCharacter>(GetAvatarActorFromActorInfo());
	if (!Character || !AM_Attack)
	{
		EndAbility(Handle,ActorInfo,ActivationInfo,true,true);
		return;
	}

	// 1타 콤보
	bIsSecondCombo = false;
	bComboWindowOpen = false;
	HitActors.Empty();
	
	// Server
	if (ActorInfo->IsNetAuthority())
	{
		GetAbilitySystemComponentFromActorInfo()->AbilityTargetDataSetDelegate(Handle,ActivationInfo.GetActivationPredictionKey()).AddUObject(this,&UGA_Attack::OnTargetDataReceived);
	}
	
	CurrentMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this,NAME_None,AM_Attack,1.f,ComboSection_First);
	CurrentMontageTask->OnCompleted.AddDynamic(this,&UGA_Attack::K2_EndAbility);
	CurrentMontageTask->OnInterrupted.AddDynamic(this,&UGA_Attack::K2_EndAbility);
	CurrentMontageTask->OnCancelled.AddDynamic(this,&UGA_Attack::K2_EndAbility);
	CurrentMontageTask->ReadyForActivation();
}

void UGA_Attack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	AGSCharacter* Character = Cast<AGSCharacter>(GetAvatarActorFromActorInfo());
	
	// Server
	if (ActorInfo->IsNetAuthority())
	{
		GetAbilitySystemComponentFromActorInfo()->AbilityTargetDataSetDelegate(Handle,ActivationInfo.GetActivationPredictionKey()).RemoveAll(this);
	}
	
	HitActors.Empty();
	
	CurrentMontageTask = nullptr;
	bIsSecondCombo = false;
	bComboWindowOpen = false;
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Attack::OnAttackTraceHit(AActor* HitActor, const FHitResult& Hit)
{
	if (!HitActor || HitActor == GetAvatarActorFromActorInfo() || HitActors.Contains(HitActor))
	{
		return;
	}
	
	HitActors.Add(HitActor);
	
	FGameplayAbilityTargetDataHandle TargetDataHandle;
	FGameplayAbilityTargetData_ActorArray* TargetData = new FGameplayAbilityTargetData_ActorArray();
	TargetData->TargetActorArray.Add(HitActor);
	TargetDataHandle.Add(TargetData);
	
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		return;
	}
	ASC->CallServerSetReplicatedTargetData(
		GetCurrentAbilitySpecHandle(),GetCurrentActivationInfo().GetActivationPredictionKey(),TargetDataHandle,FGameplayTag::EmptyTag,ASC->ScopedPredictionKey);
}

void UGA_Attack::OnComboWindowOpen()
{
	bComboWindowOpen = true;
}

// Server Outgoing to GE
void UGA_Attack::OnTargetDataReceived(const FGameplayAbilityTargetDataHandle& TargetData, FGameplayTag ActivationTag)
{
	// UE_LOG(LogTemp, Warning, TEXT("[GA_Attack][Server] OnTargetDataReceived"));
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
				SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(),TargetASC);
			}
		}
		
	}
	
	
}
