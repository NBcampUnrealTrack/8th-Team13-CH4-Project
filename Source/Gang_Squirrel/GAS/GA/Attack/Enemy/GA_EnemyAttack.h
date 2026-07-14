#pragma once

#include "CoreMinimal.h"
#include "Gang_Squirrel/GAS/GA/GA_AbilityBase.h"
#include "Gang_Squirrel/GAS/GA/Attack/IGA_AttackTraceInterface.h"
#include "GA_EnemyAttack.generated.h"

class AGS_Enemy;

UCLASS()
class GANG_SQUIRREL_API UGA_EnemyAttack : public UGA_AbilityBase,public IGA_AttackTraceInterface
{
	GENERATED_BODY()
	
public:
	UGA_EnemyAttack();
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	// Attack Interface
	virtual void OnAttackTraceHit(AActor* HitActor) override;
	virtual void OnComboWindowOpen() override;
	
	
	//TODO:: Refac to DataAssets
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Animation|Combat")
	TObjectPtr<UAnimMontage> AM_Attack;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="GameplayEffect")
	TSubclassOf<UGameplayEffect> GE_Damage;
	
private:
	void ApplyDamageToTarget(AActor* TargetActor);
	
	UPROPERTY()
	TSet<AActor*> HitActors;
};
