#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GS_EnemyAnimInst.generated.h"

class UCharacterMovementComponent;
class AGS_Enemy;

UCLASS()
class GANG_SQUIRREL_API UGS_EnemyAnimInst : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
	TObjectPtr<AGS_Enemy> OwnerCharacter;

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
