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
	
protected:
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Animation|Death")
	TObjectPtr<UAnimMontage> AM_Death;
	
private:
	UFUNCTION()
	void HandleDeathBlendOut();
};
