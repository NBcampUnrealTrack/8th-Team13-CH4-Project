#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "GS_Enemy.generated.h"

struct FGameplayTag;
class UGA_EnemyDeath;
class UGA_EnemyAttack;
class USphereComponent;
class UGameplayAbility;
class UGS_PlayerAttributeSet;

UENUM()
enum class EHandCombatType : uint8
{
	RightCombatHand,
	LeftCombatHand
};



UCLASS()
class GANG_SQUIRREL_API AGS_Enemy : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AGS_Enemy();
	
#pragma region Virtual Func
	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
#pragma endregion 
	
#pragma region Getter
	FORCEINLINE TSubclassOf<UGA_EnemyAttack> GetGA_Attack() const { return GA_Attack; }
	USphereComponent* GetCombatCollision(EHandCombatType HandType) const;
#pragma endregion 
	
#pragma region Setter
	void SetRotationTarget(AActor* NewTarget, float NewInterpSpeed);
#pragma endregion 
	
#pragma region AnimationSettingFunc
	UFUNCTION(NetMulticast,Reliable)
	void NetMultiCast_FreezeDeathPose();
#pragma endregion 
	
protected:
	
#pragma region GAS
	//ASC,AttributeSet
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly)
	TObjectPtr<UAbilitySystemComponent> EnemyAbilitySystemComp;
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly)
	TObjectPtr<UGS_PlayerAttributeSet> EnemyAttributeSet;
	// GA
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="GAS|GameplayAbility")
	TSubclassOf<UGA_EnemyAttack> GA_Attack;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="GAS|GameplayAbility")
	TSubclassOf<UGA_EnemyDeath> GA_Death;
	
private:
	// GA_Death Callback Func
	void OnDeathStateTagChanged(const FGameplayTag Tag, int32 NewCount);
#pragma endregion
	
#pragma region CombatComp
	//TODO::Need to Custom CollisionChannel
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> SphereComp_LeftHand;
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> SphereComp_RightHand;
#pragma endregion 
	
private:
#pragma region RotationValue
	UPROPERTY(Replicated)
	TObjectPtr<AActor> RotationTarget;
	UPROPERTY(Replicated)
	float RotationInterpSpeed = 30.f;
#pragma endregion 
};

