#include "GA_Attack.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Components/SphereComponent.h"
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
		return;
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
	AGSCharacter* Character = Cast<AGSCharacter>(GetAvatarActorFromActorInfo());
	
	if (Character)
	{
		EnableAttackCollision(Character,false);
	}
	HitActors.Empty();
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

// When Attack Started
void UGA_Attack::OnAttackOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp,Warning,TEXT("Overlap - HasAuthority : %s"),GetOwningActorFromActorInfo()->HasAuthority() ? TEXT("True") : TEXT("False"));
	
	if (!GetOwningActorFromActorInfo()->HasAuthority())
	{
		return;
	}
	
	if (!OtherActor || OtherActor == GetAvatarActorFromActorInfo())
	{
		return;
	}
	if (HitActors.Contains(OtherActor))
	{
		return;
	}
	
	HitActors.Add(OtherActor);
	
	UAbilitySystemComponent* HitActorASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	
	UE_LOG(LogTemp, Warning, TEXT("HitActorASC: %s"), HitActorASC ?TEXT("Valid") : TEXT("NULL"));                                            
	UE_LOG(LogTemp, Warning, TEXT("SourceASC: %s"), SourceASC ? TEXT("Valid") : TEXT("NULL"));                                                          
	UE_LOG(LogTemp, Warning, TEXT("GE_Damage: %s"), GE_Damage ? TEXT("Valid") : TEXT("NULL"));
	
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
		FActiveGameplayEffectHandle ApplyResult = SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(),HitActorASC);
		UE_LOG(LogTemp,Warning,TEXT("ApplyResult : %s"),ApplyResult.IsValid() ? TEXT("Applied") : TEXT("Failed"));
	}
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
