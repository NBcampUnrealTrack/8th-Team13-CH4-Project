#include "GS_FallingHazard.h"

#include "Components/BoxComponent.h"
#include "Components/DecalComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

AGS_FallingHazard::AGS_FallingHazard()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	SetReplicateMovement(true);

	SetNetUpdateFrequency(60.f);
	SetMinNetUpdateFrequency(30.f);

	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	SetRootComponent(RootScene);

	HazardMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HazardMesh"));
	HazardMesh->SetupAttachment(RootScene);
	HazardMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HazardMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	HazardMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	HazardMesh->SetRelativeLocation(FVector::ZeroVector);
	HazardMesh->SetRelativeRotation(FRotator::ZeroRotator);
	HazardMesh->SetRelativeScale3D(FVector(1.f, 1.f, 1.f));
	HazardMesh->SetCastShadow(false);

	WarningDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("WarningDecal"));
	WarningDecal->SetupAttachment(RootScene);
	WarningDecal->DecalSize = FVector(
		WarningDecalProjectionDepth,
		TrackingDecalStartSize,
		TrackingDecalStartSize
	);
	WarningDecal->SetWorldRotation(FRotator(-90.f, 0.f, 0.f));
	WarningDecal->SetHiddenInGame(true);

	DamageBox = CreateDefaultSubobject<UBoxComponent>(TEXT("DamageBox"));
	DamageBox->SetupAttachment(RootScene);
	DamageBox->SetRelativeLocation(FVector::ZeroVector);
	DamageBox->SetRelativeRotation(FRotator::ZeroRotator);
	DamageBox->SetBoxExtent(FVector(45.f, 45.f, 20.f));
	DamageBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DamageBox->SetCollisionObjectType(ECC_WorldDynamic);
	DamageBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	DamageBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	DamageBox->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	DamageBox->OnComponentBeginOverlap.AddDynamic(
		this,
		&AGS_FallingHazard::OnDamageBoxBeginOverlap
	);
}

void AGS_FallingHazard::BeginPlay()
{
	Super::BeginPlay();

	State = EGSFallingHazardState::Tracking;
	bIsFallingVisualReady = false;
	bHasValidGroundLocation = false;

	TrackingVisualElapsedTime = 0.f;
	bHasInitializedDecalLocation = false;

	if (HazardMesh)
	{
		HazardMesh->SetHiddenInGame(false);
	}

	if (WarningDecal)
	{
		WarningDecal->SetHiddenInGame(false);
	}

	if (DamageBox)
	{
		DamageBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (HasAuthority() == false)
	{
		return;
	}

	ChooseRandomHazardData();

	GetWorldTimerManager().SetTimer(
		TrackingTimerHandle,
		this,
		&AGS_FallingHazard::StartWarning,
		TrackingTime,
		false
	);

	ForceNetUpdate();
}

void AGS_FallingHazard::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateWarningDecalVisual(DeltaTime);

	if (State == EGSFallingHazardState::Falling && bIsFallingVisualReady)
	{
		UpdateFallingLocationByServerTime();
	}

	if (HasAuthority() == false)
	{
		return;
	}

	if (State == EGSFallingHazardState::Tracking)
	{
		UpdateTrackingLocation();
	}
}

void AGS_FallingHazard::SetTargetActor(AActor* InTargetActor)
{
	if (HasAuthority() == false)
	{
		return;
	}

	TargetActor = InTargetActor;
}

float AGS_FallingHazard::GetServerWorldTime() const
{
	const UWorld* World = GetWorld();
	if (IsValid(World) == false)
	{
		return 0.f;
	}

	AGameStateBase* GameState = World->GetGameState();
	if (IsValid(GameState) == false)
	{
		return World->GetTimeSeconds();
	}

	return GameState->GetServerWorldTimeSeconds();
}

void AGS_FallingHazard::UpdateTrackingLocation()
{
	if (IsValid(TargetActor) == false)
	{
		return;
	}

	FVector Direction = TargetActor->GetVelocity();
	Direction.Z = 0.f;

	if (Direction.IsNearlyZero())
	{
		Direction = TargetActor->GetActorForwardVector();
		Direction.Z = 0.f;
	}

	Direction = Direction.GetSafeNormal();

	const FVector TargetLocation = TargetActor->GetActorLocation();
	const FVector PredictLocation = TargetLocation + Direction * ForwardDistance;

	FVector FoundGroundLocation = FVector::ZeroVector;
	if (FindGroundLocation(PredictLocation, FoundGroundLocation))
	{
		GroundLocation = FoundGroundLocation;
		bHasValidGroundLocation = true;

		const FVector TrackingActorLocation =
			GroundLocation + FVector(0.f, 0.f, HeightOffset);

		SetActorLocation(TrackingActorLocation);
	}
	else
	{
		bHasValidGroundLocation = false;

		const FVector FallbackLocation =
			PredictLocation + FVector(0.f, 0.f, HeightOffset);

		SetActorLocation(FallbackLocation);

		if (WarningDecal)
		{
			WarningDecal->SetHiddenInGame(true);
		}
	}
}

void AGS_FallingHazard::UpdateFallingLocationByServerTime()
{
	if (bIsFallingVisualReady == false)
	{
		return;
	}

	if (FallingStartServerTime <= 0.f)
	{
		return;
	}

	const FVector StartLocation = FixedDropLocation;
	const FVector EndLocation = FVector(
		FixedDropLocation.X,
		FixedDropLocation.Y,
		GroundLocation.Z + GroundCheckDistance
	);

	const float TotalDistance = FMath::Abs(StartLocation.Z - EndLocation.Z);
	if (FMath::IsNearlyZero(TotalDistance))
	{
		SetActorLocation(EndLocation);

		if (HasAuthority())
		{
			FinishImpact();
		}

		return;
	}

	const float FallDuration = TotalDistance / FMath::Max(FallSpeed, KINDA_SMALL_NUMBER);
	const float ElapsedTime = GetServerWorldTime() - FallingStartServerTime;
	const float Alpha = FMath::Clamp(ElapsedTime / FallDuration, 0.f, 1.f);

	const FVector NewLocation = FMath::Lerp(StartLocation, EndLocation, Alpha);
	SetActorLocation(NewLocation);

	if (HasAuthority() && Alpha >= 1.f)
	{
		FinishImpact();
	}
}

bool AGS_FallingHazard::FindGroundLocation(const FVector& InLocation, FVector& OutGroundLocation) const
{
	const UWorld* World = GetWorld();
	if (IsValid(World) == false)
	{
		return false;
	}

	const FVector Start = InLocation + FVector(0.f, 0.f, GroundTraceUpOffset);
	const FVector End = InLocation - FVector(0.f, 0.f, GroundTraceDownOffset);

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	if (IsValid(TargetActor))
	{
		Params.AddIgnoredActor(TargetActor);
	}

	const bool bHit = World->LineTraceSingleByChannel(
		HitResult,
		Start,
		End,
		GroundTraceChannel,
		Params
	);

	if (bHit)
	{
		OutGroundLocation = HitResult.ImpactPoint;
		return true;
	}

	return false;
}

void AGS_FallingHazard::StartWarning()
{
	if (HasAuthority() == false)
	{
		return;
	}

	if (State != EGSFallingHazardState::Tracking)
	{
		return;
	}

	if (bHasValidGroundLocation == false)
	{
		FVector FoundGroundLocation = FVector::ZeroVector;
		if (FindGroundLocation(GetActorLocation(), FoundGroundLocation))
		{
			GroundLocation = FoundGroundLocation;
			bHasValidGroundLocation = true;
		}
		else
		{
			Destroy();
			return;
		}
	}

	FixedDropLocation = GetActorLocation();

	GroundLocation.X = FixedDropLocation.X;
	GroundLocation.Y = FixedDropLocation.Y;

	State = EGSFallingHazardState::Warning;
	bIsFallingVisualReady = false;

	SetActorLocation(FixedDropLocation);

	MulticastStartWarningVisual(FixedDropLocation, GroundLocation);

	GetWorldTimerManager().SetTimer(
		WarningTimerHandle,
		this,
		&AGS_FallingHazard::StartFalling,
		WarningTime,
		false
	);

	ForceNetUpdate();
}

void AGS_FallingHazard::StartFalling()
{
	if (HasAuthority() == false)
	{
		return;
	}

	if (State != EGSFallingHazardState::Warning)
	{
		return;
	}

	FallingStartServerTime = GetServerWorldTime();
	bIsFallingVisualReady = true;

	State = EGSFallingHazardState::Falling;

	SetActorLocation(FixedDropLocation);
	SetReplicateMovement(false);

	if (HazardMesh)
	{
		HazardMesh->SetHiddenInGame(false);
	}

	if (WarningDecal)
	{
		WarningDecal->SetHiddenInGame(false);
	}

	if (DamageBox)
	{
		DamageBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}

	MulticastStartFallingVisual(
		FixedDropLocation,
		GroundLocation,
		FallingStartServerTime
	);

	ForceNetUpdate();
}

void AGS_FallingHazard::FinishImpact()
{
	if (HasAuthority() == false)
	{
		return;
	}

	if (State == EGSFallingHazardState::Impact)
	{
		return;
	}

	State = EGSFallingHazardState::Impact;
	bIsFallingVisualReady = false;

	if (DamageBox)
	{
		DamageBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	const FVector ImpactLocation = FVector(
		FixedDropLocation.X,
		FixedDropLocation.Y,
		GroundLocation.Z + GroundCheckDistance
	);

	SetActorLocation(ImpactLocation);

	MulticastFinishImpactVisual(GroundLocation);

	UKismetSystemLibrary::PrintString(
		this,
		TEXT("Falling Hazard Impact!"),
		true,
		true,
		FLinearColor::Red,
		1.f
	);

	GetWorldTimerManager().SetTimer(
		DestroyTimerHandle,
		this,
		&AGS_FallingHazard::DestroySelf,
		ImpactDestroyDelay,
		false
	);

	ForceNetUpdate();
}

void AGS_FallingHazard::DestroySelf()
{
	if (HasAuthority() == false)
	{
		return;
	}

	Destroy();
}

void AGS_FallingHazard::MulticastStartWarningVisual_Implementation(
	FVector InFixedDropLocation,
	FVector InGroundLocation
)
{
	FixedDropLocation = InFixedDropLocation;
	GroundLocation = InGroundLocation;

	State = EGSFallingHazardState::Warning;
	bIsFallingVisualReady = false;

	SetReplicateMovement(true);
	SetActorLocation(FixedDropLocation);

	if (WarningDecal)
	{
		WarningDecal->SetHiddenInGame(false);
	}
}

void AGS_FallingHazard::MulticastStartFallingVisual_Implementation(
	FVector InFixedDropLocation,
	FVector InGroundLocation,
	float InFallingStartServerTime
)
{
	FixedDropLocation = InFixedDropLocation;
	GroundLocation = InGroundLocation;
	FallingStartServerTime = InFallingStartServerTime;

	State = EGSFallingHazardState::Falling;
	bIsFallingVisualReady = true;

	SetActorLocation(FixedDropLocation);
	SetReplicateMovement(false);

	if (WarningDecal)
	{
		WarningDecal->SetHiddenInGame(false);
	}
}

void AGS_FallingHazard::MulticastFinishImpactVisual_Implementation(FVector InGroundLocation)
{
	GroundLocation = InGroundLocation;

	State = EGSFallingHazardState::Impact;
	bIsFallingVisualReady = false;

	const FVector ImpactLocation = FVector(
		FixedDropLocation.X,
		FixedDropLocation.Y,
		GroundLocation.Z + GroundCheckDistance
	);

	SetActorLocation(ImpactLocation);

	if (DamageBox)
	{
		DamageBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (WarningDecal)
	{
		WarningDecal->SetHiddenInGame(false);
		WarningDecal->SetWorldLocation(GroundLocation + FVector(0.f, 0.f, WarningDecalZOffset));
		WarningDecal->SetWorldRotation(FRotator(-90.f, CurrentShadowDecalRotationYaw, 0.f));
		
		const float DecalSizeY = FallingDecalEndSize * CurrentShadowDecalSizeRatio.X;
		const float DecalSizeZ = FallingDecalEndSize * CurrentShadowDecalSizeRatio.Y;

		WarningDecal->DecalSize = FVector(
			WarningDecalProjectionDepth,
			DecalSizeY,
			DecalSizeZ
		);
		WarningDecal->MarkRenderStateDirty();
	}
}

void AGS_FallingHazard::OnDamageBoxBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	if (HasAuthority() == false)
	{
		return;
	}

	if (State != EGSFallingHazardState::Falling)
	{
		return;
	}

	if (IsValid(OtherActor) == false || OtherActor == this)
	{
		return;
	}

	if (HitActors.Contains(OtherActor))
	{
		return;
	}

	HitActors.Add(OtherActor);

	OnHitTarget(OtherActor);
}

void AGS_FallingHazard::OnHitTarget(AActor* HitActor)
{
	if (HasAuthority() == false)
	{
		return;
	}

	if (IsValid(HitActor) == false)
	{
		return;
	}

	UKismetSystemLibrary::PrintString(
		this,
		FString::Printf(TEXT("Falling Hazard Hit: %s"), *HitActor->GetName()),
		true,
		true,
		FLinearColor::Yellow,
		1.5f
	);

	// TODO: GE 또는 데미지 처리
}

void AGS_FallingHazard::UpdateWarningDecalVisual(float DeltaTime)
{
	if (WarningDecal == nullptr)
	{
		return;
	}

	if (State == EGSFallingHazardState::Impact)
	{
		return;
	}

	const bool bShouldShowDecal =
		State == EGSFallingHazardState::Tracking ||
		State == EGSFallingHazardState::Warning ||
		State == EGSFallingHazardState::Falling;

	WarningDecal->SetHiddenInGame(!bShouldShowDecal);

	if (bShouldShowDecal == false)
	{
		return;
	}

	FVector TargetDecalLocation = FVector::ZeroVector;

	if (State == EGSFallingHazardState::Tracking)
	{
		TargetDecalLocation = GetActorLocation() - FVector(0.f, 0.f, HeightOffset);
		TargetDecalLocation.Z += WarningDecalZOffset;
	}
	else
	{
		TargetDecalLocation = GroundLocation + FVector(0.f, 0.f, WarningDecalZOffset);
	}

	if (bHasInitializedDecalLocation == false)
	{
		CurrentDecalVisualLocation = TargetDecalLocation;
		bHasInitializedDecalLocation = true;
	}
	else
	{
		CurrentDecalVisualLocation = FMath::VInterpTo(
			CurrentDecalVisualLocation,
			TargetDecalLocation,
			DeltaTime,
			WarningDecalLocationInterpSpeed
		);
	}

	WarningDecal->SetWorldLocation(CurrentDecalVisualLocation);
	WarningDecal->SetWorldRotation(FRotator(-90.f, CurrentShadowDecalRotationYaw, 0.f));

	float CurrentSize = TrackingDecalStartSize;

	if (State == EGSFallingHazardState::Tracking)
	{
		TrackingVisualElapsedTime += DeltaTime;

		const float TrackingAlpha = FMath::Clamp(
			TrackingVisualElapsedTime / FMath::Max(TrackingTime, KINDA_SMALL_NUMBER),
			0.f,
			1.f
		);

		CurrentSize = FMath::Lerp(
			TrackingDecalStartSize,
			TrackingDecalEndSize,
			TrackingAlpha
		);
	}
	else if (State == EGSFallingHazardState::Warning)
	{
		CurrentSize = TrackingDecalEndSize;
	}
	else if (State == EGSFallingHazardState::Falling)
	{
		const float FallAlpha = GetFallingAlpha();

		CurrentSize = FMath::Lerp(
			TrackingDecalEndSize,
			FallingDecalEndSize,
			FallAlpha
		);
	}

	const float DecalSizeY = CurrentSize * CurrentShadowDecalSizeRatio.X;
	const float DecalSizeZ = CurrentSize * CurrentShadowDecalSizeRatio.Y;

	WarningDecal->DecalSize = FVector(
		WarningDecalProjectionDepth,
		DecalSizeY,
		DecalSizeZ
	);

	WarningDecal->MarkRenderStateDirty();
}

float AGS_FallingHazard::GetFallingAlpha() const
{
	if (State != EGSFallingHazardState::Falling && State != EGSFallingHazardState::Impact)
	{
		return 0.f;
	}

	const float StartZ = FixedDropLocation.Z;
	const float EndZ = GroundLocation.Z + GroundCheckDistance;
	const float CurrentZ = GetActorLocation().Z;

	const float TotalDistance = StartZ - EndZ;

	if (FMath::IsNearlyZero(TotalDistance))
	{
		return 1.f;
	}

	const float CurrentDistance = StartZ - CurrentZ;
	const float Alpha = CurrentDistance / TotalDistance;

	return FMath::Clamp(Alpha, 0.f, 1.f);
}

void AGS_FallingHazard::ChooseRandomHazardData()
{
	if (HasAuthority() == false)
	{
		return;
	}

	if (FallingHazardDataAsset == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("FallingHazardDataAsset is nullptr."));
		return;
	}

	const int32 DataCount = FallingHazardDataAsset->HazardVisualDatas.Num();
	if (DataCount <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("HazardVisualDatas is empty."));
		return;
	}

	SelectedHazardIndex = FMath::RandRange(0, DataCount - 1);
	ApplyHazardDataByIndex(SelectedHazardIndex);
}

void AGS_FallingHazard::ApplyHazardDataByIndex(int32 InIndex)
{
	if (FallingHazardDataAsset == nullptr)
	{
		return;
	}

	if (FallingHazardDataAsset->HazardVisualDatas.IsValidIndex(InIndex) == false)
	{
		return;
	}

	const FGSFallingHazardVisualData& VisualData =
		FallingHazardDataAsset->HazardVisualDatas[InIndex];

	if (HazardMesh && VisualData.Mesh)
	{
		HazardMesh->SetStaticMesh(VisualData.Mesh);
		HazardMesh->SetRelativeLocation(VisualData.MeshRelativeLocation);
		HazardMesh->SetRelativeRotation(VisualData.MeshRelativeRotation);
		HazardMesh->SetRelativeScale3D(VisualData.MeshRelativeScale);
	}

	if (DamageBox)
	{
		DamageBox->SetBoxExtent(VisualData.DamageBoxExtent);
		DamageBox->SetRelativeLocation(VisualData.DamageBoxRelativeLocation);
	}

	if (WarningDecal)
	{
		if (VisualData.ShadowDecalMaterial)
		{
			WarningDecal->SetDecalMaterial(VisualData.ShadowDecalMaterial);
		}

		CurrentShadowDecalSizeRatio = VisualData.ShadowDecalSizeRatio;
		CurrentShadowDecalRotationYaw = VisualData.ShadowDecalRotationYaw;
	}

	GroundCheckDistance = VisualData.GroundCheckDistanceOverride;
}

void AGS_FallingHazard::OnRep_SelectedHazardIndex()
{
	ApplyHazardDataByIndex(SelectedHazardIndex);
}

void AGS_FallingHazard::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGS_FallingHazard, State);
	DOREPLIFETIME(AGS_FallingHazard, GroundLocation);
	DOREPLIFETIME(AGS_FallingHazard, FixedDropLocation);
	DOREPLIFETIME(AGS_FallingHazard, FallingStartServerTime);
	DOREPLIFETIME(AGS_FallingHazard, SelectedHazardIndex);
}