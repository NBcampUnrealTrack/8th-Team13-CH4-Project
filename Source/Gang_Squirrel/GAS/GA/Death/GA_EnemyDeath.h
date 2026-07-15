#pragma once

#include "CoreMinimal.h"
#include "Gang_Squirrel/GAS/GA/GA_AbilityBase.h"
#include "GA_EnemyDeath.generated.h"


UCLASS()
class GANG_SQUIRREL_API UGA_EnemyDeath : public UGA_AbilityBase
{
	GENERATED_BODY()
	
public:
	UGA_EnemyDeath();
	
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Death|Ragdoll")
	float DeathImpulseStrength = 300.f;
};
