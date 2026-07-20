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
	bIsSecondCombo = false;
	bWillComboToSecondAttack = FMath::FRand() < ComboContinueChance;
	
	
	if (AM_Attack)
	{
		CurrentMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this,NAME_None,AM_Attack,1.f,ComboSection_First);
		
		CurrentMontageTask->OnCompleted.AddDynamic(this,&UGA_EnemyAttack::K2_EndAbility);
		CurrentMontageTask->OnCancelled.AddDynamic(this,&UGA_EnemyAttack::K2_EndAbility);
		CurrentMontageTask->OnInterrupted.AddDynamic(this,&UGA_EnemyAttack::K2_EndAbility);
		
		CurrentMontageTask->ReadyForActivation();
	}
}

void UGA_EnemyAttack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	AGS_Enemy* Enemy = Cast<AGS_Enemy>(GetAvatarActorFromActorInfo());
	
	HitActors.Empty();
	CurrentMontageTask = nullptr;
	bIsSecondCombo = false;
	bWillComboToSecondAttack = false;
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_EnemyAttack::OnAttackTraceHit(AActor* HitActor, const FHitResult& Hit)
{
	if (!HitActor || HitActor == GetAvatarActorFromActorInfo() || HitActors.Contains(HitActor))
	{
		return;
	}
	
	HitActors.Add(HitActor);
	ApplyDamageToTarget(HitActor,Hit);
}

void UGA_EnemyAttack::OnComboWindowOpen()
{
	if (!bWillComboToSecondAttack || !CurrentMontageTask || bIsSecondCombo)
	{
		return;
	}
	
	bIsSecondCombo = true;
	HitActors.Empty();
	
	GetAbilitySystemComponentFromActorInfo()->CurrentMontageJumpToSection(ComboSection_Second);
}

void UGA_EnemyAttack::ApplyDamageToTarget(AActor* TargetActor, const FHitResult& Hit)
{
	if (!GE_Damage || !TargetActor)
	{
		return;
	}

	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();

	FGameplayEffectContextHandle GEContextHandle = SourceASC->MakeEffectContext();
	GEContextHandle.AddSourceObject(this);

	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(GE_Damage,1.f,GEContextHandle);

	if (!SpecHandle.IsValid())
	{
		return;
	}

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	
	if (IsSameTeam(GetAvatarActorFromActorInfo(),TargetActor))
	{
		return;
	}
	
	if (TargetActor && TargetASC)
	{
		FActiveGameplayEffectHandle AppliedHandle = SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(),TargetASC);
		
		if (AppliedHandle.WasSuccessfullyApplied())
		{
			if (IGS_RagdollReactorInterface* TargetReactor = Cast<IGS_RagdollReactorInterface>(TargetActor))
			{
				const FName HitBone = (Hit.BoneName != NAME_None) ? Hit.BoneName : TargetReactor->GetRagdollStartBone();
				FVector ImpulseDir = !Hit.ImpactNormal.IsNearlyZero() ? -Hit.ImpactNormal : GetAvatarActorFromActorInfo()->GetActorForwardVector();
				ImpulseDir.Z = 0.f;
				ImpulseDir = ImpulseDir.GetSafeNormal(UE_KINDA_SMALL_NUMBER, GetAvatarActorFromActorInfo()->GetActorForwardVector());

				if (bIsSecondCombo)
				{
					TargetReactor->Applyknockdown(ImpulseDir * StrongHitImpulseStrength, HitBone, KnockdownDuration);
				}
				else
				{
					TargetReactor->NetMulticast_ApplyRagdollImpulse(ImpulseDir * HitImpulseStrength, HitBone);
				}
			}
		}
	}
}
