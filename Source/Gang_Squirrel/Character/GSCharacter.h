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

UCLASS()
class GANG_SQUIRREL_API AGSCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AGSCharacter();

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
	
	UFUNCTION(Server, Reliable)
	void Server_NotifyFoodEaten(AGSFoodBase* EatenFood);

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
