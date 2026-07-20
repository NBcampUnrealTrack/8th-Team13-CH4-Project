#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "Gang_Squirrel/Character/Interface/GS_RagdollReactorInterface.h"
#include "Gang_Squirrel/DataBase/DataTable/DT_Enemy.h"
#include "GS_Enemy.generated.h"

class AGS_EnemySpawnManager;
struct FOnAttributeChangeData;
class UWidgetComponent;
struct FGameplayTag;
class UGA_EnemyDeath;
class UGA_EnemyAttack;
class USphereComponent;
class UGameplayAbility;
class UGS_PlayerAttributeSet;
class AGS_PlayerState;
class USoundBase;

UCLASS()
class GANG_SQUIRREL_API AGS_Enemy : public ACharacter, public IAbilitySystemInterface, public IGS_RagdollReactorInterface
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

#pragma region RotationSettings
public:
	void SetRotationTarget(AActor* NewTarget, float NewInterpSpeed);
	// for Patrol TaskNode
	void SetRotationTarget(const FVector& NewLocation, float NewInterpSpeed);
	FORCEINLINE FVector GetHomeLocation() const {return HomeLocation;}
private:
	UPROPERTY(Replicated)
	TObjectPtr<AActor> RotationTarget;
	UPROPERTY(Replicated)
	float RotationInterpSpeed = 30.f;
	// For Patrol TaskNode
	UPROPERTY(Replicated)
	bool bRotationTargetIsLocation = false;
	UPROPERTY(Replicated)
	FVector RotationTargetLocation = FVector::ZeroVector;
#pragma endregion 
	
#pragma region Patrol
private:
	UPROPERTY()
	FVector HomeLocation = FVector::ZeroVector;
#pragma endregion 
	
#pragma region AnimationSettingFunc
public:
	UFUNCTION(NetMulticast,Reliable)
	void NetMultiCast_FreezeDeathPose();
#pragma endregion 

#pragma region GAS
protected:
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
public:
	FORCEINLINE TSubclassOf<UGA_EnemyAttack> GetGA_Attack() const { return GA_Attack; }
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
	
#pragma region DataTable
protected:
	UPROPERTY(EditDefaultsOnly,Category="Data")
	FDataTableRowHandle EnemyDataRow;
public:
	FORCEINLINE const FGS_EnemyDataTable& GetEnemyData() const {return CachedEnemyData;}
private:
	FGS_EnemyDataTable CachedEnemyData;
#pragma endregion 

#pragma region HPBar
private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UWidgetComponent> HPBarWidget;
	
	void RefreshHPBar();
	void OnHealthAttributeChanged(const FOnAttributeChangeData& Data);
#pragma endregion 

#pragma region KillCount
public:
	FORCEINLINE void SetKillerPlayerState(AGS_PlayerState* InKiller);
	
	FORCEINLINE AGS_PlayerState* GetKillerPlayerState() const { return KillerPlayerState; }

private:
	UPROPERTY()
	TObjectPtr<AGS_PlayerState> KillerPlayerState;

#pragma endregion
	
#pragma region PhysicsAnim
	UPROPERTY(EditDefaultsOnly,Category="Ragdoll|UpperBody")
	FName RagdollStartBone = TEXT("Spine02");
	UPROPERTY(EditDefaultsOnly,Category="Ragdoll|UpperBody")
	FName RagdollCollisionProfile = TEXT("Ragdoll");
	
	void SetupUpperBodyRagdoll();
	
public:	
	UFUNCTION(NetMulticast,Reliable)
	void NetMulticast_ApplyRagdollImpulse(FVector Impulse, FName BoneName) override;
	UFUNCTION(NetMulticast,Reliable)
	void NetMulticast_SetFullRagdollEnable(bool bEnable) override;

	void Applyknockdown(FVector Impulse, FName BoneName, float Duration) override;

	FORCEINLINE FName GetRagdollStartBone() const override {return RagdollStartBone;}
	FORCEINLINE FVector GetLastHitImpulseDirection() const override {return LastHitImpulseDirection;}
	FORCEINLINE void SetLastHitImpulseDirection(const FVector& Direction) override {LastHitImpulseDirection = Direction;}
private: 
	FVector LastHitImpulseDirection = FVector::ZeroVector;
	FVector DefaultMeshRelativeLocation = FVector::ZeroVector;
	FRotator DefaultMeshRelativeRotation = FRotator::ZeroRotator;
	
	FTimerHandle KnockdownRecoveryTimerHandle;

	void RecoverFromKnockdown();
	void RepositionCapsuleToRagdoll();
#pragma endregion 
	
#pragma region Pooling
public:
	void ActivateEnemy(const FVector& SpawnLocation, const FRotator& SpawnRotation);
	void DeactivateEnemy();
	void ScheduleReturnToPool(float Delay);
	
	FORCEINLINE bool IsPoolActive() const {return bIsPoolActive;}
	void SetOwningSpawnManager(AGS_EnemySpawnManager* InManager);
	
private:
	UPROPERTY(ReplicatedUsing = OnRep_IsPoolActive)
	bool bIsPoolActive = false;
	
	UFUNCTION()
	void OnRep_IsPoolActive();
	
	void InitializeFromDataTable();
	bool bAbilitiesGranted = false;
	
	UPROPERTY()
	TWeakObjectPtr<AGS_EnemySpawnManager> OwningSpawnManager;
	
	FTimerHandle ReturnToPoolTimerHandle;
	void ReturnToPoolDeferred();
#pragma endregion 

#pragma region Sound
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Death")
	TObjectPtr<USoundBase> DeathSound;

public:
	UFUNCTION(NetMulticast, Unreliable)
	void NetMulticast_PlayDeathSound();
#pragma endregion
};

