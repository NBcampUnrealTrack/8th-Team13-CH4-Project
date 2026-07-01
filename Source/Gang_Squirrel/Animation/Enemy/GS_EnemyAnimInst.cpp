#include "GS_EnemyAnimInst.h"
#include "Gang_Squirrel/Character/GSCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Gang_Squirrel/Enemy/GS_Enemy.h"
#include "Kismet/KismetMathLibrary.h"

void UGS_EnemyAnimInst::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	APawn* OwnerPawn = TryGetPawnOwner();
	if (IsValid(OwnerPawn) == true)
	{
		OwnerCharacter = Cast<AGS_Enemy>(OwnerPawn);

		if (IsValid(OwnerCharacter) == true)
		{
			OwnerCharacterMovement = OwnerCharacter->GetCharacterMovement();
		}
	}
}

void UGS_EnemyAnimInst::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (IsValid(OwnerCharacter) == false)
	{
		APawn* OwnerPawn = TryGetPawnOwner();
		OwnerCharacter = Cast<AGS_Enemy>(OwnerPawn);
	}

	if (IsValid(OwnerCharacter) == false)
	{
		return;
	}

	if (IsValid(OwnerCharacterMovement) == false)
	{
		OwnerCharacterMovement = OwnerCharacter->GetCharacterMovement();
	}

	if (IsValid(OwnerCharacterMovement) == false)
	{
		return;
	}

	Velocity = OwnerCharacterMovement->Velocity;
	GroundSpeed = UKismetMathLibrary::VSizeXY(Velocity);

	const float GroundAcceleration = UKismetMathLibrary::VSizeXY(OwnerCharacterMovement->GetCurrentAcceleration());
	const bool bIsAccelerationNearlyZero = FMath::IsNearlyZero(GroundAcceleration);

	bShouldMove = (KINDA_SMALL_NUMBER < GroundSpeed) && (bIsAccelerationNearlyZero == false);
	bIsFalling = OwnerCharacterMovement->IsFalling();

	const FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
	const FVector RightVector = OwnerCharacter->GetActorRightVector();

	FVector NormalizedVelocity = FVector(Velocity.X, Velocity.Y, 0.f);
	NormalizedVelocity.Normalize();

	const float ForwardValue = FVector::DotProduct(ForwardVector, NormalizedVelocity);
	const float RightValue = FVector::DotProduct(RightVector, NormalizedVelocity);

	Direction = FMath::RadiansToDegrees(FMath::Atan2(RightValue, ForwardValue));
}
