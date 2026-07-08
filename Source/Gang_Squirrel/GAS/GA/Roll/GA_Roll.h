#pragma once

#include "CoreMinimal.h"
#include "Gang_Squirrel/GAS/GA/GA_AbilityBase.h"
#include "GA_Roll.generated.h"

class UGameplayEffect;
class UAnimMontage;

UCLASS()
class GANG_SQUIRREL_API UGA_Roll : public UGA_AbilityBase
{
	GENERATED_BODY()

public:
	UGA_Roll();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Roll")
	float RollStaminaCost = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameplayEffect")
	TSubclassOf<UGameplayEffect> GE_RollStamina;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TObjectPtr<UAnimMontage> AM_Roll;

private:
	float GetCurrentStamina() const;
};