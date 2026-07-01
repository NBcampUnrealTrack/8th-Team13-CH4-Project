// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "GSCharacter.generated.h"

class UGA_Attack;
class UCameraComponent;
class USpringArmComponent;
class UInputMappingContext;
class UInputAction;
class USphereComponent;
class UWidgetComponent;
class UAnimMontage;

UCLASS()
class GANG_SQUIRREL_API AGSCharacter : public ACharacter
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
	
	FORCEINLINE USphereComponent* GetLeftHandCollision() const {return leftHandCollision;}
	FORCEINLINE USphereComponent* GetRightHandCollision() const {return rightHandCollision;}
	

private:
	//InputAction Function
	void IAMove(const FInputActionValue& InValue);

	void IALook(const FInputActionValue& InValue);

	void IAInteract(const FInputActionValue& InValue);

	void IAAttack(const FInputActionValue& InValue);

	void IAStartSprint(const FInputActionValue& InValue);

	void IAEndSprint(const FInputActionValue& InValue);

	void IARolling(const FInputActionValue& InValue);


private:
	//Sprint Function
	void SetSprinting(bool bNewSprinting);

	UFUNCTION(Server, Reliable)
	void ServerSetSprinting(bool bNewSprinting);


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


protected:
	//Feature
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Sprint")
	float WalkSpeed = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Sprint")
	float SprintSpeed = 900.f;

	UPROPERTY(BlueprintReadOnly, Category = "Movement|Sprint")
	uint8 bIsSprinting : 1 = false;

#pragma region Animation
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Roll")
	TObjectPtr<UAnimMontage> AM_Roll;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Roll")
	float RollingDistance = 450.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Roll")
	float RollingDuration = 0.6f;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Roll")
	uint8 bIsRolling : 1 = false;

	UPROPERTY()
	FVector RollingDirection = FVector::ZeroVector;

	float RollingElapsedTime = 0.f;

	void StartRolling();
	void FinishRolling();
#pragma endregion

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

#pragma endregion
	
#pragma region GA
protected:
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="GameplayAbility")
	TSubclassOf<UGA_Attack> GA_Attack;
#pragma endregion 
};
