#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GS_FallingHazardDataAsset.h"
#include "GS_FallingHazard.generated.h"

class USceneComponent;
class UBoxComponent;
class UStaticMeshComponent;
class UDecalComponent;
class UGameplayEffect;

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

	void SetTargetActor(AActor* InTargetActor);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<USceneComponent> RootScene;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UStaticMeshComponent> HazardMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UBoxComponent> DamageBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UDecalComponent> WarningDecal;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Falling Hazard|Data")
	TObjectPtr<UGS_FallingHazardDataAsset> FallingHazardDataAsset;

	UPROPERTY(ReplicatedUsing = OnRep_SelectedHazardIndex)
	int32 SelectedHazardIndex = INDEX_NONE;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling Hazard|Tracking")
	float ForwardDistance = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling Hazard|Tracking")
	float HeightOffset = 120.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling Hazard|Time")
	float TrackingTime = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling Hazard|Time")
	float WarningTime = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling Hazard|Falling")
	float FallSpeed = 450.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling Hazard|Falling")
	float ImpactDestroyDelay = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Falling Hazard|Damage")
	TSubclassOf<UGameplayEffect> GE_Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling Hazard|Trace")
	float GroundTraceUpOffset = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling Hazard|Trace")
	float GroundTraceDownOffset = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling Hazard|Trace")
	float GroundCheckDistance = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling Hazard|Trace")
	TEnumAsByte<ECollisionChannel> GroundTraceChannel = ECC_GameTraceChannel1;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling Hazard|Warning")
	float WarningDecalZOffset = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling Hazard|Warning")
	float WarningDecalProjectionDepth = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling Hazard|Warning")
	float TrackingDecalStartSize = 40.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling Hazard|Warning")
	float TrackingDecalEndSize = 110.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling Hazard|Warning")
	float FallingDecalEndSize = 120.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling Hazard|Warning")
	float WarningDecalLocationInterpSpeed = 18.f;

	UPROPERTY()
	FVector2D CurrentShadowDecalLocationOffset = FVector2D::ZeroVector;

	UPROPERTY()
	FVector2D CurrentShadowDecalSizeRatio = FVector2D(1.f, 1.f);

	UPROPERTY()
	float CurrentShadowDecalRotationYaw = 0.f;

	float TrackingVisualElapsedTime = 0.f;

	uint8 bHasInitializedDecalLocation : 1 = false;

	FVector CurrentDecalVisualLocation = FVector::ZeroVector;

protected:
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Falling Hazard")
	EGSFallingHazardState State = EGSFallingHazardState::Tracking;

	UPROPERTY()
	TObjectPtr<AActor> TargetActor;

	UPROPERTY(Replicated)
	FVector GroundLocation = FVector::ZeroVector;

	UPROPERTY(Replicated)
	FVector FixedDropLocation = FVector::ZeroVector;

	UPROPERTY(Replicated)
	float FallingStartServerTime = 0.f;

	bool bHasValidGroundLocation = false;
	bool bIsFallingVisualReady = false;
	bool bCanDamage = false;

	UPROPERTY()
	TSet<TObjectPtr<AActor>> HitActors;

	FTimerHandle TrackingTimerHandle;
	FTimerHandle WarningTimerHandle;
	FTimerHandle DestroyTimerHandle;

protected:
	float GetServerWorldTime() const;

	void UpdateTrackingLocation();
	void UpdateFallingLocationByServerTime();

	bool FindGroundLocation(const FVector& InLocation, FVector& OutGroundLocation) const;

	void StartWarning();
	void StartFalling();
	void FinishImpact();
	void DestroySelf();

	void OnHitTarget(AActor* HitActor);

	void UpdateWarningDecalVisual(float DeltaTime);
	float GetFallingAlpha() const;

	void ChooseRandomHazardData();
	void ApplyHazardDataByIndex(int32 InIndex);

	UFUNCTION()
	void OnRep_SelectedHazardIndex();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastStartWarningVisual(FVector InFixedDropLocation, FVector InGroundLocation);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastStartFallingVisual(
		FVector InFixedDropLocation,
		FVector InGroundLocation,
		float InFallingStartServerTime
	);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFinishImpactVisual(FVector InGroundLocation);

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