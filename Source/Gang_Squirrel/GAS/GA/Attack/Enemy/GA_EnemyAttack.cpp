#include "GA_EnemyAttack.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Components/SphereComponent.h"
#include "Gang_Squirrel/Enemy/GS_Enemy.h"
#include "Gang_Squirrel/Gang_Squirrel.h"
#include "Gang_Squirrel/GAS/Tags/GS_GamePlayTag.h"

UGA_EnemyAttack::UGA_EnemyAttack()
{
	AbilityTags.AddTag(AbilityTag::TAG_Ability_Attack);
	ActivationBlockedTags.AddTag(StateTag::TAG_State_Dead);
}

void UGA_EnemyAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	AGS_Enemy* Enemy = Cast<AGS_Enemy>(GetAvatarActorFromActorInfo());
	if (!Enemy)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	HitActors.Empty();
	
	
	if (AM_Attack)
	{
		UAbilityTask_PlayMontageAndWait* TaskMontage = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this,NAME_None,AM_Attack);
		
		TaskMontage->OnCompleted.AddDynamic(this,&UGA_EnemyAttack::K2_EndAbility);
		TaskMontage->OnCancelled.AddDynamic(this,&UGA_EnemyAttack::K2_EndAbility);
		TaskMontage->OnInterrupted.AddDynamic(this,&UGA_EnemyAttack::K2_EndAbility);
		
		TaskMontage->ReadyForActivation();
	}
}

void UGA_EnemyAttack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	AGS_Enemy* Enemy = Cast<AGS_Enemy>(GetAvatarActorFromActorInfo());
	
	HitActors.Empty();
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_EnemyAttack::OnAttackTraceHit(AActor* HitActor, const FHitResult& Hit)
{
	if (!HitActor || HitActor == GetAvatarActorFromActorInfo() || HitActors.Contains(HitActor))
	{
		return;
	}
	
	HitActors.Add(HitActor);
	ApplyDamageToTarget(HitActor);
}

void UGA_EnemyAttack::OnComboWindowOpen()
{
	UE_LOG(LogTemp,Warning,TEXT("None"));
}

void UGA_EnemyAttack::ApplyDamageToTarget(AActor* TargetActor)
{
	UE_LOG(LogGAS, Warning, TEXT("[GA_EnemyAttack] ApplyDamageToTarget - Target:%s, GE_Damage:%s"), *GetNameSafe(TargetActor), GE_Damage ? *GE_Damage->GetName() : TEXT("NULL"));

	if (!GE_Damage || !TargetActor)
	{
		return;
	}

	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();

	FGameplayEffectContextHandle GEContextHandle = SourceASC->MakeEffectContext();
	GEContextHandle.AddSourceObject(this);

	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(GE_Damage,1.f,GEContextHandle);

	UE_LOG(LogGAS, Warning, TEXT("[GA_EnemyAttack] ApplyDamageToTarget - SpecHandle.IsValid:%s"), SpecHandle.IsValid() ? TEXT("true") : TEXT("false"));

	if (!SpecHandle.IsValid())
	{
		return;
	}

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	
	if (IsSameTeam(GetAvatarActorFromActorInfo(),TargetActor))
	{
		return;
	}

	UE_LOG(LogGAS, Warning, TEXT("[GA_EnemyAttack] ApplyDamageToTarget - TargetASC:%s"), TargetASC ? TEXT("valid") : TEXT("NULL"));

	if (TargetActor && TargetASC)
	{
		FActiveGameplayEffectHandle AppliedHandle = SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(),TargetASC);
		UE_LOG(LogGAS, Warning, TEXT("[GA_EnemyAttack] ApplyDamageToTarget - AppliedHandle.WasSuccessfullyApplied:%s"), AppliedHandle.WasSuccessfullyApplied() ? TEXT("true") : TEXT("false"));
	}
}
