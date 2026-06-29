#include "GA_Attack.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Gang_Squirrel/Character/GSCharacter.h"

UGA_Attack::UGA_Attack()
{
}

void UGA_Attack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	AGSCharacter* Character = Cast<AGSCharacter>(GetAvatarActorFromActorInfo());
	if (!Character)
	{
		EndAbility(Handle,ActorInfo,ActivationInfo,true,true);
	}
	HitActors.Empty();
	
	EnableAttackCollision(Character,true);
	
	// TODO::Make AttackMontage
	if (AM_Attack)
	{
		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,NAME_None,AM_Attack);
		
		MontageTask->OnCompleted.AddDynamic(this, &UGA_Attack::K2_EndAbility);
		MontageTask->OnInterrupted.AddDynamic(this, &UGA_Attack::K2_EndAbility);
		MontageTask->OnCancelled.AddDynamic(this, &UGA_Attack::K2_EndAbility);
		
		MontageTask->ReadyForActivation();
	}
	
}

void UGA_Attack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

// When Attack Started
void UGA_Attack::OnAttackOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == GetAvatarActorFromActorInfo())
	{
		return;
	}
	if (HitActors.Contains(OtherActor))
	{
		return;
	}
	
	UAbilitySystemComponent* HitActorASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	
	if (!HitActorASC || !GE_Damage || !SourceASC)
	{
		return;
	}
	
	// Who make GE
	FGameplayEffectContextHandle ContextHandle = SourceASC->MakeEffectContext();
	ContextHandle.AddSourceObject(this);
	
	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(GE_Damage,1.f,ContextHandle);
	
	if (SpecHandle.IsValid())
	{
		SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(),HitActorASC);
	}
}

void UGA_Attack::EnableAttackCollision(AGSCharacter* OwnerCharacter, bool bEnable)
{
}
