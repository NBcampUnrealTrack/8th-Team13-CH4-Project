#include "GS_FallingHazard.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TimerManager.h"

AGS_FallingHazard::AGS_FallingHazard()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	HazardMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HazardMesh"));
	HazardMesh->SetupAttachment(SceneRoot);
	HazardMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HazardMesh->SetRelativeScale3D(FVector(3.f, 3.f, 0.5f));


	DamageBox = CreateDefaultSubobject<UBoxComponent>(TEXT("DamageBox"));
	DamageBox->SetupAttachment(HazardMesh);
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
	DamageBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GetWorldTimerManager().SetTimer(
		TrackingTimerHandle,
		this,
		&AGS_FallingHazard::StartWarning,
		TrackingTime,
		false
	);
}

void AGS_FallingHazard::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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
	TargetActor = InTargetActor;
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
		ECC_Visibility,
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
}

void AGS_FallingHazard::StartFalling()
{
	if (State != EGSFallingHazardState::Warning)
	{
		return;
	}

	State = EGSFallingHazardState::Falling;

	SetActorLocation(FixedDropLocation);

	HazardMesh->SetHiddenInGame(false);

	DamageBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AGS_FallingHazard::FinishImpact()
{
	if (State == EGSFallingHazardState::Impact)
	{
		return;
	}

	State = EGSFallingHazardState::Impact;

	DamageBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	SetActorLocation(GroundLocation + FVector(0.f, 0.f, GroundCheckDistance));

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
}

void AGS_FallingHazard::DestroySelf()
{
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

	// GAS 담당자가 나중에 이 함수 안에 GE 적용 로직을 연결하면 됨.
	// 또는 인터페이스 방식으로 피격/음식 손실 처리를 연결해도 됨.
}