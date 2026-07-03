#pragma once

#include "CoreMinimal.h"
#include "Gang_Squirrel/GAS/GA/GA_AbilityBase.h"
#include "GA_PlayerDeath.generated.h"

UCLASS()
class GANG_SQUIRREL_API UGA_PlayerDeath : public UGA_AbilityBase
{
	GENERATED_BODY()
	
public:
	UGA_PlayerDeath();
	
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Animation|Death")
	TObjectPtr<UAnimMontage> AM_Death;
	//TODO:: TempLogic -> Refac to Respawn Logic
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Death|Respawn")
	float RespawnDelay = 3.f;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Death|Respawn")
	TSubclassOf<UGameplayEffect> GE_Respawn;
	
private:
	//TODO:: TempLogic -> Refac to Respawn Logic
	void HandleRespawn();
	
	// For AnimMontage Stop
	UFUNCTION()
	void HandleDeathBlendOut();
	
	FVector CachedRespawnLocation = FVector::ZeroVector;
	FRotator CachedRespawnRotation = FRotator::ZeroRotator;
	FTimerHandle RespawnTimerHandle;
};
