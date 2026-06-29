#pragma once

#include "CoreMinimal.h"
#include "Gang_Squirrel/GAS/GA/GA_AbilityBase.h"
#include "GA_Attack.generated.h"

class AGSCharacter;

UCLASS()
class GANG_SQUIRREL_API UGA_Attack : public UGA_AbilityBase
{
	GENERATED_BODY()
	
public:
	UGA_Attack();
	
protected:
	// GA Override Func
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
	//TODO::Refac to DataAssets
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Animation|Combat")
	TObjectPtr<UAnimMontage> AM_Attack;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="GameplayEffect")
	TSubclassOf<UGameplayEffect> GE_Damage;
	
private:
	// AttackCollision Func
	UFUNCTION()
	void OnAttackOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,bool bFromSweep, const FHitResult& SweepResult);
	
	void EnableAttackCollision(AGSCharacter* OwnerCharacter, bool bEnable);
	void OnTargetDataReceived(const FGameplayAbilityTargetDataHandle& TargetData, FGameplayTag ActivationTag);
	
	UPROPERTY()
	TSet<AActor*> HitActors;
};
