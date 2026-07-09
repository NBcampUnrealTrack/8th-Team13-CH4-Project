// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "GameplayEffectTypes.h"
#include "GSCharacter.generated.h"

class UGA_PlayerDeath;
struct FGameplayTag;
class UGA_Attack;
class UGA_Roll;
class UGA_Sprint;
class UCameraComponent;
class USpringArmComponent;
class UInputMappingContext;
class UInputAction;
class USphereComponent;
class UWidgetComponent;
class UAnimMontage;
class UGS_StaminaBarWidget;

UCLASS()
class GANG_SQUIRREL_API AGSCharacter : public ACharacter ,public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AGSCharacter();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

public:

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	//ASC Connection
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	FORCEINLINE USphereComponent* GetLeftHandCollision() const {return leftHandCollision;}
	FORCEINLINE USphereComponent* GetRightHandCollision() const {return rightHandCollision;}
	

private:
	//InputAction Function
	void IAMove(const FInputActionValue& InValue);

	void IALook(const FInputActionValue& InValue);

	void IAInteract(const FInputActionValue& InValue);
	
	void IAStopInteract(const FInputActionValue& InValue);

	void IAAttack(const FInputActionValue& InValue);

	void IAStartSprint(const FInputActionValue& InValue);

	void IAEndSprint(const FInputActionValue& InValue);

	void IARolling(const FInputActionValue& InValue);


private:
	//Sprint Function
	void SetSprinting(bool bNewSprinting);

public:
	void StartSprintFromAbility();
	void StopSprintFromAbility();
	void RollFromAbility();

public:
	UFUNCTION()
	void UpdateNameTag(const FString& Newname);

protected:

	//Components
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Component")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UCameraComponent> Camera;


	//InputAction
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> IMC;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> Move;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> Look;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> Interact;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> Attack;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> Sprint;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> Rolling;

public:
	
	//Food
	UFUNCTION(Server, Reliable)
	void Server_NotifyFoodEaten(AGSFoodBase* EatenFood);
	
	UFUNCTION(Server, Reliable)
	void Server_SetEating(bool bEating);
	
	void ResetCheekSize();
	
	void AddMaxCheekSize(float Value);
	
	void InflateCheeks(float Value);
	
	bool bIsEating = false;
	
protected:
	
	//Food
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_InflateCheeks(float Value);
	
private:
	
	//Food
	UPROPERTY()
	float CurrentCheekSize = 0.f;
	
	UPROPERTY()
	float MaxCheekSize = 1.f;
	
	UPROPERTY()
	float IncreasingPercent = 1.f;

public:
	UFUNCTION(BlueprintPure, Category = "Movement|Sprint")
	bool IsSprinting() const { return bIsSprinting; }

	UFUNCTION(BlueprintPure, Category = "Movement|Roll")
	bool IsRolling() const { return bIsRolling; }

private:	
	// Don't trust this Values. Go to AttributeSet
	float CachedMoveSpeed = 50.f;
	float CachedSlowSpeedMultiplier = 1.f;

	bool bMovementSpeedDelegateBound = false;
	bool bStaminaDelegateBound = false;

	void BindMovementSpeedDelegates();
	void OnMoveSpeedChanged(const FOnAttributeChangeData& Data);
	void OnSlowSpeedMultiplierChanged(const FOnAttributeChangeData& Data);

	void UpdateMaxWalkSpeedFromAttribute();
	float GetFinalMoveSpeedMultiplier() const;


protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Sprint")
	float SprintSpeedMultiplier = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Roll")
	float RollSpeedMultiplier = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Roll")
	float RollingDuration = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Roll")
	TObjectPtr<UAnimMontage> AM_Roll;

	void StartRolling(const FVector& InRollingDirection);

	UFUNCTION(Server, Reliable)
	void ServerStartRolling(FVector_NetQuantizeNormal InRollingDirection);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayRollMontage();

private:
	UPROPERTY(BlueprintReadOnly, Category = "Movement|Sprint", meta = (AllowPrivateAccess = "true"))
	uint8 bIsSprinting : 1 = false;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Roll", meta = (AllowPrivateAccess = "true"))
	uint8 bIsRolling : 1 = false;

	FTimerHandle RollingTimerHandle;

	FVector RollingDirection = FVector::ZeroVector;
	
	void StartRollingLocal(const FVector& InRollingDirection);
	void FinishRolling();
	FVector GetRollingDirection() const;

#pragma region Component

protected:
	//left right hand Collision Component 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Component")
	TObjectPtr<USphereComponent> leftHandCollision;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Component")
	TObjectPtr<USphereComponent> rightHandCollision;

	//Head Up Widget
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UWidgetComponent> PlayerNameTagWidget;

	//Side Widget
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UWidgetComponent> StaminaBarWidget;

#pragma endregion

#pragma region StaminaWidget

	protected:
		void BindStaminaDelegates();
		void UpdateStaminaBar(float CurrentStamina, float MaxStamina);
		void RefreshStaminaBarVisibility(float CurrentStamina, float MaxStamina);

		void OnStaminaChanged(const FOnAttributeChangeData& Data);
		void OnMaxStaminaChanged(const FOnAttributeChangeData& Data);

		float CachedMaxStamina = 100.f;

		FTimerHandle StaminaBarHideTimerHandle;
		void HideStaminaBar();

#pragma endregion

#pragma region GA
public:
	UFUNCTION(NetMulticast,Reliable)
	void NetMulticast_SetDeathPoseFrozen(bool bFrozen);
	
protected:
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="GameplayAbility")
	TSubclassOf<UGA_Attack> GA_Attack;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameplayAbility")
	TSubclassOf<UGA_Roll> GA_Roll;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="GameplayAbility")
	TSubclassOf<UGA_PlayerDeath> GA_Death;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameplayAbility")
	TSubclassOf<UGA_Sprint> GA_Sprint;
	// GA_Death CallBack Func
private:
	void OnDeathStateTagChanged(const FGameplayTag Tag, int32 NewCount);

#pragma endregion 
};
