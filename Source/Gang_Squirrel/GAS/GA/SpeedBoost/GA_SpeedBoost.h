// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gang_Squirrel/GAS/GA/GA_AbilityBase.h"
#include "GA_SpeedBoost.generated.h"

/**
 * 
 */

class AGSCharacter;

UCLASS()
class GANG_SQUIRREL_API UGA_SpeedBoost : public UGA_AbilityBase
{
	GENERATED_BODY()
	
public:
    UGA_SpeedBoost();

protected:
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* ActorInfo,const FGameplayAbilityActivationInfo ActivationInfo,const FGameplayEventData* TriggerEventData) override;

    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* ActorInfo,const FGameplayAbilityActivationInfo ActivationInfo,bool bReplicateEndAbility,bool bWasCancelled) override;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpeedBoost")
    float SpeedMultiplier = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpeedBoost")
    float Duration = 5.f;

private:
    void HandleDurationExpired();

    FTimerHandle SpeedBoostTimerHandle;
};
