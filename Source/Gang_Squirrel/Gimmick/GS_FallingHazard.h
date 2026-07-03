#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GS_FallingHazard.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class UDecalComponent;

UENUM(BlueprintType)
enum class EGSFallingHazardState : uint8
{
	Tracking,
	Warning,
	Falling,
	Impact
};


UCLASS()
class GANG_SQUIRREL_API AGS_FallingHazard : public ACharacter
{
	GENERATED_BODY()

public:
	AGS_FallingHazard();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	void SetTargetActor(AActor* InTargetActor);

protected:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UStaticMeshComponent> HazardMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UBoxComponent> DamageBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UDecalComponent> WarningDecal;


protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling Hazard|Tracking")
	float ForwardDistance = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling Hazard|Tracking")
	float HeightOffset = 300.f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling Hazard|Time")
	float TrackingTime = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling Hazard|Time")
	float WarningTime = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling Hazard|Falling")
	float FallSpeed = 1500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling Hazard|Falling")
	float ImpactDestroyDelay = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling Hazard|Trace")
	float GroundTraceUpOffset = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling Hazard|Trace")
	float GroundTraceDownOffset = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling Hazard|Trace")
	float GroundCheckDistance = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling Hazard|Trace")
	TEnumAsByte<ECollisionChannel> GroundTraceChannel = ECC_GameTraceChannel1;

#pragma region Decal

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling Hazard|Warning")
	FVector WarningDecalSize = FVector(128.f, 300.f, 300.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling Hazard|Warning")
	float WarningDecalZOffset = 5.f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling Hazard|Warning")
	float WarningDecalProjectionDepth = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling Hazard|Warning")
	float TrackingDecalStartSize = 120.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling Hazard|Warning")
	float TrackingDecalEndSize = 420.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling Hazard|Warning")
	float FallingDecalEndSize = 450.f;

	UPROPERTY(Replicated)
	float TrackingStartServerTime = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling Hazard|Warning")
	float WarningDecalLocationInterpSpeed = 18.f;

	float TrackingVisualElapsedTime = 0.f;

	uint8 bHasInitializedDecalLocation : 1 = false;

	FVector CurrentDecalVisualLocation = FVector::ZeroVector;
#pragma endregion
	
protected:
	
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Falling Hazard")
	EGSFallingHazardState State = EGSFallingHazardState::Tracking;


	UPROPERTY()
	TObjectPtr<AActor> TargetActor;

	UPROPERTY(Replicated)
	FVector GroundLocation = FVector::ZeroVector;

	UPROPERTY(Replicated)
	FVector FixedDropLocation = FVector::ZeroVector;

	UPROPERTY()
	TSet<TObjectPtr<AActor>> HitActors;

	FTimerHandle TrackingTimerHandle;
	FTimerHandle WarningTimerHandle;
	FTimerHandle DestroyTimerHandle;

protected:
	float GetServerWorldTime() const;

	void UpdateTrackingLocation();
	bool FindGroundLocation(const FVector& InLocation, FVector& OutGroundLocation) const;

	void StartWarning();
	void StartFalling();
	void FinishImpact();
	void DestroySelf();

	void OnHitTarget(AActor* HitActor);

	void UpdateWarningDecalVisual(float DeltaTime);

	float GetFallingAlpha() const;

	UFUNCTION()
	void OnDamageBoxBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};