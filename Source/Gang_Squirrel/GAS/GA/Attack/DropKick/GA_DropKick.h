#pragma once

#include "CoreMinimal.h"
#include "Gang_Squirrel/GAS/GA/GA_AbilityBase.h"
#include "Gang_Squirrel/GAS/GA/Attack/IGA_AttackTraceInterface.h"
#include "GA_DropKick.generated.h"

class UAbilityTask_PlayMontageAndWait;

UCLASS()
class GANG_SQUIRREL_API UGA_DropKick : public UGA_AbilityBase, public  IGA_AttackTraceInterface
{
	GENERATED_BODY()
	
public:
	UGA_DropKick();
	
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
	virtual void OnAttackTraceHit(AActor* HitActor, const FHitResult& Hit) override;
	virtual void OnComboWindowOpen() override {};
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Animation")
	TObjectPtr<UAnimMontage> AM_DropKick;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="GameplayEffect")
	TSubclassOf<UGameplayEffect> GE_Damage;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="PhysicsReaction")
	float KnockdownImpulseStrength = 40.f;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="PhysicsReaction")
	float KnockdownDuration = 1.5f;
	
private:
	void OnTargetReceived(const FGameplayAbilityTargetDataHandle& TargetData, FGameplayTag ActivationTag);
	
	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> CurrentMontageTask;
	UPROPERTY()
	TSet<AActor*> HitActors;
};
