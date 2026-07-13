#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GSAnimInstance.generated.h"

class AGSCharacter;
class UCharacterMovementComponent;

UCLASS()
class GANG_SQUIRREL_API UGSAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
	TObjectPtr<AGSCharacter> OwnerCharacter;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
	TObjectPtr<UCharacterMovementComponent> OwnerCharacterMovement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	FVector Velocity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	float GroundSpeed = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	float Direction = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	uint8 bShouldMove : 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	uint8 bIsFalling : 1;

};