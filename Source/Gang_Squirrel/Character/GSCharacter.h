// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "GameplayEffectTypes.h"
#include "GSCharacter.generated.h"

class UGSSlideWidget;
class UGA_PlayerDeath;
struct FGameplayTag;
class UGA_Attack;
class UGA_Roll;
class UGA_Sprint;
class UGA_Grab;
class UCameraComponent;
class USpringArmComponent;
class UInputMappingContext;
class UInputAction;
class USphereComponent;
class UWidgetComponent;
class UAnimMontage;
class UGS_StaminaBarWidget;
class UGSCheekWidget;
class UGSFoodWidget;
class UGameplayEffect;

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
	
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	//InputAction Function
	void IAMove(const FInputActionValue& InValue);

	void IAStopMove(const FInputActionValue& InValue);

	void IALook(const FInputActionValue& InValue);

	void IAInteract(const FInputActionValue& InValue);
	
	void IAStopInteract(const FInputActionValue& InValue);

	void IAAttack(const FInputActionValue& InValue);

	void IAStartSprint(const FInputActionValue& InValue);

	void IAEndSprint(const FInputActionValue& InValue);

	void IARolling(const FInputActionValue& InValue);

	void IAStartGrab(const FInputActionValue& InValue);

	void IAEndGrab(const FInputActionValue& InValue);


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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> Grab;

#pragma region Fall

protected:
	virtual void Landed(const FHitResult& Hit) override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FallDamage", meta = (AllowPrivateAccess = "true"))
	float FallDamageVelocityThreshold = 900.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FallDamage", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UGameplayEffect> GE_FallDamage;

	float MaxFallSpeedDuringFall = 0.f;

#pragma endregion

#pragma region Grab

private:

	UPROPERTY()
	FVector LastMoveInputWorldDirection = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab", meta = (AllowPrivateAccess = "true"))
	float GrabPullStrength = 8.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab", meta = (AllowPrivateAccess = "true"))
	float MaxGrabResistance = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab", meta = (AllowPrivateAccess = "true"))
	float GrabResistanceSpeedPenalty = 0.5f;

	UPROPERTY()
	FVector LastGrabberLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab|Push", meta = (AllowPrivateAccess = "true"))
	float GrabPushDistanceMultiplier = 1.0f;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetGrabAnimation(bool bGrab);

	float CurrentGrabResistance = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab|Push", meta = (AllowPrivateAccess = "true"))
	float GrabPushContactDistance = 18.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab|Push", meta = (AllowPrivateAccess = "true"))
	float GrabAlignmentStrength = 8.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab|Push", meta = (AllowPrivateAccess = "true"))
	float MaxGrabCorrectionSpeed = 120.f;

	UFUNCTION(Server, Unreliable)
	void ServerSetMoveInputDirection(FVector_NetQuantizeNormal InMoveDirection);

protected:

	UPROPERTY(Replicated)
	uint8 bIsGrabbing : 1 = false;

	UPROPERTY(Replicated)
	uint8 bIsGrabbed : 1 = false;

	UPROPERTY(Replicated)
	TObjectPtr<AGSCharacter> GrabbedTarget;

	UPROPERTY(Replicated)
	TObjectPtr<AGSCharacter> GrabOwner;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Grab")
	TObjectPtr<UAnimMontage> AM_Grab;

	UFUNCTION(Server, Reliable)
	void ServerCancelGrabAbility();

	void CancelGrabAbility();

public:

	FVector GetLastMoveInputWorldDirection() const { return LastMoveInputWorldDirection; }

	float GetGrabResistanceAgainst(const FVector& PushDirection) const;

	void UpdateGrabTargetPosition(float DeltaTime);

	bool IsGrabbing() const { return bIsGrabbing; }
	bool IsGrabbed() const { return bIsGrabbed; }

	void StartGrabTarget(AGSCharacter* TargetCharacter);
	void StopGrab();

	float GetCurrentGrabResistance() const { return CurrentGrabResistance; }


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab|Push", meta = (AllowPrivateAccess = "true"))
	float MinGrabPushMultiplier = 0.25f;

#pragma endregion

#pragma region Food,Cheek
	
public:
	
	//Food
	UFUNCTION(Server, Reliable)
	void Server_NotifyFoodEaten(AGSFoodBase* EatenFood);
	
	UFUNCTION(Server, Reliable)
	void Server_NotifyAddScore(int32 Value);
	
	UFUNCTION(Server, Reliable)
	void Server_SetEating(bool bEating);
	
	void ResetCheekSize();
	
	void AddMaxCheekSize(float Value);
	
	void InflateCheeks(float Value);
	
	void AddTempScore(int32 Value);
	
	void ResetTempScore();
	
	void UpdateSlideWidget(int32 Value);
	
	FORCEINLINE int32 GetTempScore() { return TempScore; }
	
	bool bIsEating = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Score")
	TSubclassOf<UGSSlideWidget> SlideWidgetClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Score")
	TSubclassOf<UGSSlideWidget> SlideWidgetRewardClass;
	
protected:
	
	//Food
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_InflateCheeks(float Value);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetEatingAnimation(bool bEating);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UGSCheekWidget> CheekWidgetClass;
	
	UPROPERTY(BlueprintReadOnly, Category = "UI")
	UGSCheekWidget* CheekWidgetUIInstance;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Eat")
	TObjectPtr<UAnimMontage> AM_Eat;

private:
	
	//Food
	UPROPERTY(ReplicatedUsing = OnRep_CheekSize)
	float CurrentCheekSize = 0.f;
	
	UPROPERTY(ReplicatedUsing = OnRep_CheekSize)
	float MaxCheekSize = 1.f;
	
	UPROPERTY()
	int32 TempScore = 0;

	UFUNCTION()
	void OnRep_CheekSize();
	
#pragma endregion

private:
	//Sprint Function
	void SetSprinting(bool bNewSprinting);

public:
	void StartSprintFromAbility();
	void StopSprintFromAbility();
	void RollFromAbility();

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
		void HideStaminaBar();
		void UpdateStaminaBarWorldLocation();

		void OnStaminaChanged(const FOnAttributeChangeData& Data);
		void OnMaxStaminaChanged(const FOnAttributeChangeData& Data);

		float CachedMaxStamina = 100.f;

		FTimerHandle StaminaBarHideTimerHandle;

	protected:
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Stamina")
		float StaminaBarSideOffset = 6.f;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Stamina")
		float StaminaBarHeightOffset = -2.f;

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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameplayAbility")
	TSubclassOf<UGameplayAbility> GA_Grab;
	// GA_Death CallBack Func
private:
	void OnDeathStateTagChanged(const FGameplayTag Tag, int32 NewCount);


#pragma endregion 
};
