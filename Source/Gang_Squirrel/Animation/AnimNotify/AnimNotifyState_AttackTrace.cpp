#include "AnimNotifyState_AttackTrace.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Gang_Squirrel/GAS/GA/Attack/IGA_AttackTraceInterface.h"
#include "Gang_Squirrel/GAS/Tags/GS_GamePlayTag.h"
#include "Gang_Squirrel/Gang_Squirrel.h"

void UAnimNotifyState_AttackTrace::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                               float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	// UE_LOG(LogGAS, Warning, TEXT("[AttackTrace] NotifyBegin - MeshComp:%s, Owner:%s, HasAuthority:%s"),
	// 	*GetNameSafe(MeshComp), MeshComp ? *GetNameSafe(MeshComp->GetOwner()) : TEXT("N/A"),
	// 	(MeshComp && MeshComp->GetOwner()) ? (MeshComp->GetOwner()->HasAuthority() ? TEXT("true") : TEXT("false")) : TEXT("N/A"));

	if (MeshComp)
	{
		PrevSocketLocationMap.Add(MeshComp,MeshComp->GetSocketLocation(TraceSocketName));
	}
}

void UAnimNotifyState_AttackTrace::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	// UE_LOG(LogGAS, Warning, TEXT("[AttackTrace] NotifyTick - MeshComp:%s, TraceSocketName:%s"), *GetNameSafe(MeshComp), *TraceSocketName.ToString());

	AActor* OwnerActor = MeshComp ? MeshComp->GetOwner() : nullptr;
	if (!OwnerActor || !OwnerActor->HasAuthority())
	{
		// UE_LOG(LogGAS, Warning, TEXT("[AttackTrace] NotifyTick - OwnerActor:%s, HasAuthority:%s (return)"), *GetNameSafe(OwnerActor), OwnerActor ? (OwnerActor->HasAuthority() ? TEXT("true") : TEXT("false")) : TEXT("N/A"));
		return;
	}

	const FVector CurrentSocketLocation = MeshComp->GetSocketLocation(TraceSocketName);

	TArray<FHitResult> HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerActor);

	FVector& PrevSocketLocation = PrevSocketLocationMap.FindOrAdd(MeshComp,CurrentSocketLocation);

	// Owner Scale(Z)만큼 TraceRadius도 같이 축소/확대 (소켓 위치는 이미 스케일 반영되어 있지만 반경은 별도 보정 필요)
	const float ScaleMultiplier = OwnerActor->GetActorScale3D().Z;
	const float ScaledTraceRadius = TraceRadius * ScaleMultiplier;

	const bool bHit = MeshComp->GetWorld()->SweepMultiByChannel(
		HitResult,PrevSocketLocation,CurrentSocketLocation,FQuat::Identity,ECC_Pawn,FCollisionShape::MakeSphere(ScaledTraceRadius),QueryParams);

	// UE_LOG(LogGAS, Warning, TEXT("[AttackTrace] NotifyTick - Prev:%s, Current:%s, Radius:%f, bHit:%s, NumHits:%d"),
	// 	*PrevSocketLocation.ToString(), *CurrentSocketLocation.ToString(), ScaledTraceRadius, bHit ? TEXT("true") : TEXT("false"), HitResult.Num());

	if (bDrawDebug)
	{
		DrawDebugSphere(MeshComp->GetWorld(), CurrentSocketLocation,ScaledTraceRadius,12,bHit ? FColor::Red : FColor::Green,false, 1.f);
	}

	PrevSocketLocation = CurrentSocketLocation;

	if (!bHit)
	{
		return;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerActor);
	if (!ASC)
	{
		UE_LOG(LogGAS, Error, TEXT("[AttackTrace] NotifyTick - ASC is NULL for OwnerActor:%s"), *GetNameSafe(OwnerActor));
		return;
	}

	UGameplayAbility* ActiveAttackAbility = nullptr;
	for (FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
	{
		// UE_LOG(LogGAS, Warning, TEXT("[AttackTrace] NotifyTick - Checking Spec Ability:%s, IsActive:%s"),
		// 	Spec.Ability ? *Spec.Ability->GetClass()->GetName() : TEXT("NULL"), Spec.IsActive() ? TEXT("true") : TEXT("false"));

		if (Spec.IsActive() && Spec.Ability && Spec.Ability->AbilityTags.HasTag(AbilityTag::TAG_Ability_Attack))
		{
			ActiveAttackAbility = Spec.GetPrimaryInstance();
			break;
		}
	}

	// UE_LOG(LogGAS, Warning, TEXT("[AttackTrace] NotifyTick - ActiveAttackAbility:%s"), *GetNameSafe(ActiveAttackAbility));

	IGA_AttackTraceInterface* TraceInstigator = Cast<IGA_AttackTraceInterface>(ActiveAttackAbility);
	if (!TraceInstigator)
	{
		// UE_LOG(LogGAS, Error, TEXT("[AttackTrace] NotifyTick - Cast to IGA_AttackTraceInterface FAILED"));
		return;
	}

	for (const FHitResult& Hit : HitResult)
	{
		if (AActor* HitActor = Hit.GetActor())
		{
			// UE_LOG(LogGAS, Warning, TEXT("[AttackTrace] NotifyTick - OnAttackTraceHit calling with HitActor:%s"), *GetNameSafe(HitActor));
			TraceInstigator->OnAttackTraceHit(HitActor);
		}
	}

}

void UAnimNotifyState_AttackTrace::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	PrevSocketLocationMap.Remove(MeshComp);
}
