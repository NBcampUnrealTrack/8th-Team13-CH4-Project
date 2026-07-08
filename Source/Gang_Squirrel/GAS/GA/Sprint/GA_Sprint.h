// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gang_Squirrel/GAS/GA/GA_AbilityBase.h"
#include "GA_Sprint.generated.h"

class UGameplayEffect;
class AGSCharacter;

UCLASS()
class GANG_SQUIRREL_API UGA_Sprint : public UGA_AbilityBase
{
	GENERATED_BODY()

public:
	UGA_Sprint();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled
	) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sprint")
	float MinStaminaToSprint = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sprint")
	float StaminaCostInterval = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameplayEffect")
	TSubclassOf<UGameplayEffect> GE_SprintStaminaCost;

private:
	void ApplySprintCost();
	float GetCurrentStamina() const;

	FTimerHandle SprintCostTimerHandle;
};