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
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Death|Respawn")
	float RespawnDelay = 10.f;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Death|Respawn")
	TSubclassOf<UGameplayEffect> GE_Respawn;
	
	//Physics AnimProperty
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Death|Ragdoll")
	float DeathImpulseStrength = 300.f;
	
private:
	void HandleRespawn();
	
	FVector CachedRespawnLocation = FVector::ZeroVector;
	FRotator CachedRespawnRotation = FRotator::ZeroRotator;
	FTimerHandle RespawnTimerHandle;
};
