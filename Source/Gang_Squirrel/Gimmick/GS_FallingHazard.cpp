#include "GS_FallingHazard.h"

#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "Components/DecalComponent.h"
#include "GameFramework/GameStateBase.h"


AGS_FallingHazard::AGS_FallingHazard()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	SetReplicateMovement(true);

	SetNetUpdateFrequency(30.f);
	SetMinNetUpdateFrequency(15.f);

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCapsuleHalfHeight(50.f);
	GetCapsuleComponent()->SetCapsuleRadius(50.f);
	GetCharacterMovement()->GravityScale = 0.f;
	GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	GetCharacterMovement()->bOrientRotationToMovement = false;

	HazardMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HazardMesh"));
	HazardMesh->SetupAttachment(GetRootComponent());
	HazardMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HazardMesh->SetRelativeLocation(FVector::ZeroVector);
	HazardMesh->SetRelativeRotation(FRotator::ZeroRotator);
	HazardMesh->SetRelativeScale3D(FVector(3.f, 3.f, 0.5f));
	HazardMesh->SetCastShadow(false);

	WarningDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("WarningDecal"));
	WarningDecal->DecalSize = FVector(
		WarningDecalProjectionDepth,
		TrackingDecalStartSize,
		TrackingDecalStartSize
	);
	WarningDecal->SetWorldRotation(FRotator(-90.f, 0.f, 0.f));
	WarningDecal->SetHiddenInGame(true);

	DamageBox = CreateDefaultSubobject<UBoxComponent>(TEXT("DamageBox"));
	DamageBox->SetupAttachment(GetRootComponent());
	DamageBox->SetRelativeLocation(FVector::ZeroVector);
	DamageBox->SetRelativeRotation(FRotator::ZeroRotator);
	DamageBox->SetBoxExtent(FVector(150.f, 150.f, 50.f));
	DamageBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DamageBox->SetCollisionObjectType(ECC_WorldDynamic);
	DamageBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	DamageBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	DamageBox->OnComponentBeginOverlap.AddDynamic(
		this,
		&AGS_FallingHazard::OnDamageBoxBeginOverlap
	);
}

void AGS_FallingHazard::BeginPlay()
{
	Super::BeginPlay();

	State = EGSFallingHazardState::Tracking;

	HazardMesh->SetHiddenInGame(false);

	TrackingVisualElapsedTime = 0.f;
	
	if (WarningDecal)
	{
		WarningDecal->SetHiddenInGame(false);
	}

	DamageBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	
	if (HasAuthority() == false)
	{
		return;
	}

	TrackingStartServerTime = GetServerWorldTime();

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

	if (HasAuthority() == false)
	{
		return;
	}

	if (State == EGSFallingHazardState::Tracking)
	{
		UpdateTrackingLocation();
	}
	else if (State == EGSFallingHazardState::Falling)
	{
		const FVector DeltaLocation = FVector(0.f, 0.f, -FallSpeed * DeltaTime);

	
		AddActorWorldOffset(DeltaLocation, true);

		if (GetActorLocation().Z <= GroundLocation.Z + GroundCheckDistance)
		{
			FinishImpact();
		}
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
	UWorld* World = GetWorld();
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

	FVector FoundGroundLocation;
	if (FindGroundLocation(PredictLocation, FoundGroundLocation) == true)
	{
		GroundLocation = FoundGroundLocation;

		SetActorLocation(GroundLocation + FVector(0.f, 0.f, HeightOffset));

	}
	else
	{
		SetActorLocation(PredictLocation + FVector(0.f, 0.f, HeightOffset));

		if (WarningDecal)
		{
			WarningDecal->SetHiddenInGame(true);
		}
	}
	
}

bool AGS_FallingHazard::FindGroundLocation(const FVector& InLocation, FVector& OutGroundLocation) const
{
	UWorld* World = GetWorld();
	if (IsValid(World) == false)
	{
		return false;
	}

	const FVector Start = InLocation + FVector(0.f, 0.f, GroundTraceUpOffset);
	const FVector End = InLocation - FVector(0.f, 0.f, GroundTraceDownOffset);

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	if (IsValid(TargetActor) == true)
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

	if (bHit == true)
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

	State = EGSFallingHazardState::Warning;

	FixedDropLocation = GetActorLocation();

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

	State = EGSFallingHazardState::Falling;

	SetActorLocation(FixedDropLocation);

	HazardMesh->SetHiddenInGame(false);

	if (WarningDecal)
	{
		WarningDecal->SetHiddenInGame(false);
	}

	DamageBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

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

	DamageBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	SetActorLocation(GroundLocation + FVector(0.f, 0.f, GroundCheckDistance));

	if (WarningDecal)
	{
		WarningDecal->SetHiddenInGame(false);
		WarningDecal->SetWorldLocation(GroundLocation + FVector(0.f, 0.f, WarningDecalZOffset));
		WarningDecal->SetWorldRotation(FRotator(-90.f, 0.f, 0.f));
		WarningDecal->DecalSize = FVector(
			WarningDecalProjectionDepth,
			FallingDecalEndSize,
			FallingDecalEndSize
		);
	}

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
//GE
}

void AGS_FallingHazard::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGS_FallingHazard, State);
	DOREPLIFETIME(AGS_FallingHazard, GroundLocation);
	DOREPLIFETIME(AGS_FallingHazard, FixedDropLocation);
	DOREPLIFETIME(AGS_FallingHazard, TrackingStartServerTime);
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
	WarningDecal->SetWorldRotation(FRotator(-90.f, 0.f, 0.f));

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

	const FVector NewDecalSize = FVector(
		WarningDecalProjectionDepth,
		CurrentSize,
		CurrentSize
	);

	WarningDecal->DecalSize = NewDecalSize;
	WarningDecal->MarkRenderStateDirty();
}