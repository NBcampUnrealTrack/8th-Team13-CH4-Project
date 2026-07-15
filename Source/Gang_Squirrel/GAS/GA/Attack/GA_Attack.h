#pragma once

#include "CoreMinimal.h"
#include "IGA_AttackTraceInterface.h"
#include "Gang_Squirrel/GAS/GA/GA_AbilityBase.h"
#include "GA_Attack.generated.h"

class UAbilityTask_PlayMontageAndWait;
class AGSCharacter;

UCLASS()
class GANG_SQUIRREL_API UGA_Attack : public UGA_AbilityBase, public IGA_AttackTraceInterface
{
	GENERATED_BODY()
	
public:
	UGA_Attack();
	
	void RequestComboInput();
	
protected:
	// GA Override Func
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	// InterFace PureVirtual
	virtual void OnAttackTraceHit(AActor* HitActor, const FHitResult& Hit) override;
	virtual void OnComboWindowOpen() override;
	
#pragma region CombatAnim
protected:
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Animation|Combat")
	TObjectPtr<UAnimMontage> AM_Attack;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Animation|Combat")
	FName ComboSection_First = TEXT("Attack1");
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Animation|Combat")
	FName ComboSection_Second = TEXT("Attack2");
	
private:
	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> CurrentMontageTask;
	
	bool bIsSecondCombo = false;
	bool bComboWindowOpen = false;
#pragma endregion 
	
protected:
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="GameplayEffect")
	TSubclassOf<UGameplayEffect> GE_Damage;
	
private:
	void OnTargetDataReceived(const FGameplayAbilityTargetDataHandle& TargetData, FGameplayTag ActivationTag);
	
	UPROPERTY()
	TSet<AActor*> HitActors;
};
