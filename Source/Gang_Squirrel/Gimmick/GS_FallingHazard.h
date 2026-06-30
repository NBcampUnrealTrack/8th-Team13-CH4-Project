#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GS_FallingHazard.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

UENUM(BlueprintType)
enum class EGSFallingHazardState : uint8
{
	Tracking,
	Warning,
	Falling,
	Impact
};

UCLASS()
class GANG_SQUIRREL_API AGS_FallingHazard : public AActor
{
	GENERATED_BODY()

public:
	AGS_FallingHazard();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
	void SetTargetActor(AActor* InTargetActor);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UStaticMeshComponent> HazardMesh;

	/*UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UStaticMeshComponent> WarningMesh;*/

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UBoxComponent> DamageBox;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling Hazard")
	float ForwardDistance = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling Hazard")
	float HeightOffset = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling Hazard")
	float TrackingTime = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling Hazard")
	float WarningTime = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling Hazard")
	float FallSpeed = 1500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling Hazard")
	float ImpactDestroyDelay = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling Hazard")
	float GroundTraceUpOffset = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling Hazard")
	float GroundTraceDownOffset = 3000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling Hazard")
	float GroundCheckDistance = 50.f;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Falling Hazard")
	EGSFallingHazardState State = EGSFallingHazardState::Tracking;

	UPROPERTY()
	TObjectPtr<AActor> TargetActor;

	UPROPERTY()
	FVector GroundLocation = FVector::ZeroVector;

	UPROPERTY()
	FVector FixedDropLocation = FVector::ZeroVector;

	UPROPERTY()
	TSet<TObjectPtr<AActor>> HitActors;

	FTimerHandle TrackingTimerHandle;
	FTimerHandle WarningTimerHandle;
	FTimerHandle DestroyTimerHandle;

protected:
	void UpdateTrackingLocation();
	bool FindGroundLocation(const FVector& InLocation, FVector& OutGroundLocation) const;

	void StartWarning();
	void StartFalling();
	void FinishImpact();
	void DestroySelf();

	void OnHitTarget(AActor* HitActor);

	UFUNCTION()
	void OnDamageBoxBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
};