#pragma once

#include "CoreMinimal.h"
#include "Gang_Squirrel/GAS/GA/GA_AbilityBase.h"
#include "GA_Grab.generated.h"

class UGameplayEffect;
class AGSCharacter;

UCLASS()
class GANG_SQUIRREL_API UGA_Grab : public UGA_AbilityBase
{
	GENERATED_BODY()

public:
	UGA_Grab();

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

private:
	UPROPERTY(EditAnywhere, Category = "Grab")
	float MinStaminaToGrab = 10.f;

	UPROPERTY(EditAnywhere, Category = "Grab")
	float StaminaCostInterval = 0.1f;

	UPROPERTY(EditAnywhere, Category = "Grab")
	float GrabRange = 20.f;

	UPROPERTY(EditAnywhere, Category = "Grab")
	float GrabRadius = 20.f;

	UPROPERTY(EditAnywhere, Category = "GameplayEffect")
	TSubclassOf<UGameplayEffect> GE_GrabStaminaCost;

	UPROPERTY()
	TObjectPtr<AGSCharacter> GrabberCharacter;

	UPROPERTY()
	TObjectPtr<AGSCharacter> TargetCharacter;

	FTimerHandle GrabTickTimerHandle;

	float GetCurrentStamina() const;

	AGSCharacter* FindGrabTarget(AGSCharacter* InGrabber) const;

	void GrabTick();
};