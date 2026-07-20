#include "GA_Grab.h"

#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Gang_Squirrel/Character/GSCharacter.h"
#include "Gang_Squirrel/GAS/AttributeSet/GS_PlayerAttributeSet.h"
#include "Gang_Squirrel/GAS/Tags/GS_GamePlayTag.h"

UGA_Grab::UGA_Grab()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

	// AbilityTags.AddTag(AbilityTag::TAG_Ability_Grab);

	ActivationBlockedTags.AddTag(StateTag::TAG_State_Dead);
}

void UGA_Grab::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData
)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!ActorInfo || !ActorInfo->IsNetAuthority())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	GrabberCharacter = Cast<AGSCharacter>(GetAvatarActorFromActorInfo());
	if (!IsValid(GrabberCharacter))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (GrabberCharacter->IsGrabbed())
	{
		EndAbility(
			Handle,
			ActorInfo,
			ActivationInfo,
			true,
			true
		);
		return;
	}

	if (GetCurrentStamina() < MinStaminaToGrab)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	TargetCharacter = FindGrabTarget(GrabberCharacter);
	if (!IsValid(TargetCharacter))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (CommitAbility(Handle, ActorInfo, ActivationInfo) == false)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	GrabberCharacter->StartGrabTarget(TargetCharacter);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			GrabTickTimerHandle,
			this,
			&UGA_Grab::GrabTick,
			StaminaCostInterval,
			true,
			0.f
		);
	}
}

void UGA_Grab::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled
)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(GrabTickTimerHandle);
	}

	if (ActorInfo && ActorInfo->IsNetAuthority())
	{
		if (IsValid(GrabberCharacter))
		{
			GrabberCharacter->StopGrab();
		}
	}

	GrabberCharacter = nullptr;
	TargetCharacter = nullptr;

	Super::EndAbility(
		Handle,
		ActorInfo,
		ActivationInfo,
		bReplicateEndAbility,
		bWasCancelled
	);
}

AGSCharacter* UGA_Grab::FindGrabTarget(AGSCharacter* InGrabber) const
{
	if (!IsValid(InGrabber))
	{
		return nullptr;
	}

	UWorld* World = InGrabber->GetWorld();
	if (!World)
	{
		return nullptr;
	}

	const FVector Start =
		InGrabber->GetActorLocation() + FVector(0.f, 0.f, 5.f);

	const FVector End =
		Start + InGrabber->GetActorForwardVector() * GrabRange;

	TArray<FHitResult> Hits;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(InGrabber);

	const bool bHit = World->SweepMultiByChannel(
		Hits,
		Start,
		End,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(GrabRadius),
		Params
	);

	if (!bHit)
	{
		return nullptr;
	}

	for (const FHitResult& Hit : Hits)
	{
		AGSCharacter* Candidate = Cast<AGSCharacter>(Hit.GetActor());
		if (!IsValid(Candidate))
		{
			continue;
		}

		if (Candidate == InGrabber)
		{
			continue;
		}

		if (Candidate->IsGrabbed())
		{
			continue;
		}

		return Candidate;
	}

	return nullptr;
}

void UGA_Grab::GrabTick()
{
	if (!CurrentActorInfo || !CurrentActorInfo->IsNetAuthority())
	{
		K2_EndAbility();
		return;
	}

	if (!IsValid(GrabberCharacter) || !IsValid(TargetCharacter))
	{
		K2_EndAbility();
		return;
	}

	if (GetCurrentStamina() <= 0.f)
	{
		K2_EndAbility();
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC || !GE_GrabStaminaCost)
	{
		K2_EndAbility();
		return;
	}

	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	Context.AddSourceObject(this);

	FGameplayEffectSpecHandle SpecHandle =
		ASC->MakeOutgoingSpec(GE_GrabStaminaCost, 1.f, Context);

	if (SpecHandle.IsValid())
	{
		ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}

	if (GetCurrentStamina() <= 0.f)
	{
		K2_EndAbility();
	}
}

float UGA_Grab::GetCurrentStamina() const
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ASC == nullptr)
	{
		return 0.f;
	}

	return ASC->GetNumericAttribute(
		UGS_PlayerAttributeSet::GetStaminaAttribute()
	);
}