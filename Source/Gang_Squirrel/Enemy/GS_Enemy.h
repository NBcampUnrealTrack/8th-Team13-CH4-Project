#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "GS_Enemy.generated.h"

class USphereComponent;
class UGameplayAbility;
class UGS_PlayerAttributeSet;

UENUM()
enum class EHandCombatType : uint8
{
	RightCombatHand,
	LeftCombatHand
};



//TODO::Attach AIController, BB,BT
UCLASS()
class GANG_SQUIRREL_API AGS_Enemy : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AGS_Enemy();
	
	virtual void Tick(float DeltaTime) override;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	FORCEINLINE TSubclassOf<UGameplayAbility> GetGA_Attack() const { return GA_Attack; }
	
	USphereComponent* GetCombatCollision(EHandCombatType HandType) const;
	
protected:
	virtual void BeginPlay() override;
	
#pragma region GAS
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly)
	TObjectPtr<UAbilitySystemComponent> EnemyAbilitySystemComp;
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly)
	TObjectPtr<UGS_PlayerAttributeSet> EnemyAttributeSet;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="GAS|GameplayAbility")
	TSubclassOf<UGameplayAbility> GA_Attack;
#pragma endregion
	
#pragma region CombatComp
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> SphereComp_LeftHand;
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> SphereComp_RightHand;
#pragma endregion 
};
