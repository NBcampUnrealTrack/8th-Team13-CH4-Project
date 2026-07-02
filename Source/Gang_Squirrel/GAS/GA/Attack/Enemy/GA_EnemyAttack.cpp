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
	
	if (ActorInfo->IsNetAuthority())
	{
		EnableAttackCollision(Enemy,true);
	}
	
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
	
	if (Enemy && ActorInfo->IsNetAuthority())
	{
		EnableAttackCollision(Enemy,false);
	}
	
	HitActors.Empty();
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_EnemyAttack::OnAttackOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("[GA_EnemyAttack] OnAttackOverlap - Other:%s"),*GetNameSafe(OtherActor));
	
	if (!OtherActor || OtherActor == GetAvatarActorFromActorInfo())
	{
		return;
	}
	
	if (HitActors.Contains(OtherActor))
	{
		return;
	}
	
	HitActors.Add(OtherActor);
	
	ApplyDamageToTarget(OtherActor);
}

void UGA_EnemyAttack::EnableAttackCollision(AGS_Enemy* OwnerEnemy, bool bEnable)
{
	UE_LOG(LogTemp, Warning, TEXT("[GA_EnemyAttack] EnableAttackCollision - bEnable:%s"), bEnable ? TEXT("true") : TEXT("false"));
	
	auto OnCollision = [&](USphereComponent* HandCollision)
	{
		if (!HandCollision)
		{
			UE_LOG(LogTemp, Error, TEXT("[GA_EnemyAttack] HandCollision is null!"));
			return;
		}
		
		HandCollision->SetCollisionEnabled(bEnable ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
		
		UE_LOG(LogTemp, Warning, TEXT("[GA_EnemyAttack] %s CollisionEnabled set to %d"),*HandCollision->GetName(), (int32)HandCollision->GetCollisionEnabled())
		
		if (bEnable)
		{
			HandCollision->OnComponentBeginOverlap.AddDynamic(this, &UGA_EnemyAttack::OnAttackOverlap);
		}
		else
		{
			HandCollision->OnComponentBeginOverlap.RemoveDynamic(this, &UGA_EnemyAttack::OnAttackOverlap);
		}
	};
	
	OnCollision(OwnerEnemy->GetCombatCollision(EHandCombatType::LeftCombatHand));
	OnCollision(OwnerEnemy->GetCombatCollision(EHandCombatType::RightCombatHand));
}

void UGA_EnemyAttack::ApplyDamageToTarget(AActor* TargetActor)
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
	if (TargetActor && TargetASC)
	{
		SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(),TargetASC);
	}
	
	
}
