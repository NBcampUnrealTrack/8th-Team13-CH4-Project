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

void UGA_Attack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	AGSCharacter* Character = Cast<AGSCharacter>(GetAvatarActorFromActorInfo());
	if (!Character)
	{
		EndAbility(Handle,ActorInfo,ActivationInfo,true,true);
		return;
	}
	
	HitActors.Empty();
	
	// Server
	if (ActorInfo->IsNetAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[GA_Attack][Server] ActivateAbility - Delegate Binding"));
		GetAbilitySystemComponentFromActorInfo()->AbilityTargetDataSetDelegate(Handle,ActivationInfo.GetActivationPredictionKey()).AddUObject(this, &UGA_Attack::OnTargetDataReceived);
	}
	
	// Client
	if (Character->IsLocallyControlled())
	{
		EnableAttackCollision(Character,true);
	}
	
	// AnimMontage Logic
	if (AM_Attack)
	{
		UAbilityTask_PlayMontageAndWait* TaskMontage = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this,NAME_None,AM_Attack);
		
		TaskMontage->OnCompleted.AddDynamic(this, &UGA_Attack::K2_EndAbility);
		TaskMontage->OnCancelled.AddDynamic(this, &UGA_Attack::K2_EndAbility);
		TaskMontage->OnInterrupted.AddDynamic(this, &UGA_Attack::K2_EndAbility);
		
		TaskMontage->ReadyForActivation();
	}
}

void UGA_Attack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	AGSCharacter* Character = Cast<AGSCharacter>(GetAvatarActorFromActorInfo());
	
	if (Character && Character->IsLocallyControlled())
	{
		EnableAttackCollision(Character,false);
	}
	// Server
	if (ActorInfo->IsNetAuthority())
	{
		GetAbilitySystemComponentFromActorInfo()->AbilityTargetDataSetDelegate(Handle,ActivationInfo.GetActivationPredictionKey()).RemoveAll(this);
	}
	
	HitActors.Empty();
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

// When Attack Started
void UGA_Attack::OnAttackOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("[GA_Attack][Client] OnAttackOverlap: %s"), *GetNameSafe(OtherActor));
	
	if (!OtherActor || OtherActor == GetAvatarActorFromActorInfo())
	{
		return;
	}
	
	if (HitActors.Contains(OtherActor))
	{
		return;
	}
	
	HitActors.Add(OtherActor);
	
	FGameplayAbilityTargetDataHandle TargetDataHandle;
	FGameplayAbilityTargetData_ActorArray* TargetData = new FGameplayAbilityTargetData_ActorArray();
	
	TargetData->TargetActorArray.Add(OtherActor);
	TargetDataHandle.Add(TargetData);
	
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	ASC->CallServerSetReplicatedTargetData(GetCurrentAbilitySpecHandle(), GetCurrentActivationInfo().GetActivationPredictionKey()
		,TargetDataHandle,FGameplayTag::EmptyTag,ASC->ScopedPredictionKey);
	
}

void UGA_Attack::EnableAttackCollision(AGSCharacter* OwnerCharacter, bool bEnable)
{
	// Create Lamda
	auto OnCollision = [&](USphereComponent* HandCollision)
	{
		if (!HandCollision)
		{
			return;
		}
		
		HandCollision->SetCollisionEnabled(bEnable ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
		
		if (bEnable)
		{
			HandCollision->OnComponentBeginOverlap.AddDynamic(this, &UGA_Attack::OnAttackOverlap);
		}
		else
		{
			HandCollision->OnComponentBeginOverlap.RemoveDynamic(this, &UGA_Attack::OnAttackOverlap);
		}
	};
	
	// Enable AttackHand Collision
	OnCollision(OwnerCharacter->GetLeftHandCollision());
	OnCollision(OwnerCharacter->GetRightHandCollision());
}

// Server Outgoing to GE
void UGA_Attack::OnTargetDataReceived(const FGameplayAbilityTargetDataHandle& TargetData, FGameplayTag ActivationTag)
{
	UE_LOG(LogTemp, Warning, TEXT("[GA_Attack][Server] OnTargetDataReceived"));
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
				SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(),TargetASC);
			}
		}
		
	}
	
	
}
