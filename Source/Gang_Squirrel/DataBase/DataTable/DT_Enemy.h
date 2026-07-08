#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "DT_Enemy.generated.h"


class UGameplayAbility;

USTRUCT(BlueprintType)
struct FGS_EnemyDataTable : public FTableRowBase
{
	GENERATED_BODY()
	
	// Perception
	UPROPERTY(EditAnywhere,Category="Perception")
	float SightRadius = 50.f;
	UPROPERTY(EditAnywhere,Category="Perception")
	float LoseSightRadius = 100.f;
	
	// Chase
	UPROPERTY(EditAnywhere,Category="Chase")
	float AcceptanceRadius = 30.f;
	UPROPERTY(EditAnywhere,Category="Chase")
	float ChaseRotationInterpSpeed = 30.f;
	
	// Attack
	UPROPERTY(EditAnywhere,Category="Attack")
	float FacingToleranceDegrees = 10.f;
	UPROPERTY(EditAnywhere,Category="Attack")
	float AttackRotationInterpSpeed = 30.f;
	
	// Stat
	UPROPERTY(EditAnywhere,Category="Stat")
	float Health = 3.f;
	UPROPERTY(EditAnywhere,Category="Stat")
	float MaxHealth = 3.f;
	UPROPERTY(EditAnywhere,Category="Stat")
	float MoveSpeed = 600.f;
	
	// Ability
	UPROPERTY(EditAnywhere,Category="Ability")
	TArray<TSubclassOf<UGameplayAbility>> GrantedAbilities;
	
	// Patrol
	UPROPERTY(EditAnywhere,Category="Patrol")
	float PatrolRadius = 500.f;
	UPROPERTY(EditAnywhere,Category="Patrol")
	float PatrolAcceptanceRadius = 30.f;
	UPROPERTY(EditAnywhere,Category="Patrol")
	float PatrolRotationInterpSpeed = 20.f;
	UPROPERTY(EditAnywhere,Category="Patrol")
	float PatrolWaitTime = 2.f;
};


