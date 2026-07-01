#include "GS_FallingHazard.h"

#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "Components/DecalComponent.h"

AGS_FallingHazard::AGS_FallingHazard()
{
	PrimaryActorTick.bCanEverTick = true;

	/*
	 * 이 액터 자체를 네트워크 복제 대상으로 만든다.
	 */
	bReplicates = true;

	/*
	 * 서버에서 SetActorLocation / AddActorWorldOffset 등으로 움직인 결과를
	 * 클라이언트에도 복제한다.
	 */
	SetReplicateMovement(true);

	/*
	 * ACharacter는 기본으로 CapsuleComponent를 Root로 가진다.
	 * 낙하물은 플레이어처럼 걸어다니는 캐릭터가 아니므로,
	 * Capsule 충돌은 꺼두고 DamageBox로만 판정한다.
	 */
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCapsuleHalfHeight(50.f);
	GetCapsuleComponent()->SetCapsuleRadius(50.f);

	/*
	 * CharacterMovement가 중력이나 보행 처리로 낙하물을 건드리지 않게 설정.
	 * 실제 이동은 우리가 Tick에서 직접 처리한다.
	 */
	GetCharacterMovement()->GravityScale = 0.f;
	GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	GetCharacterMovement()->bOrientRotationToMovement = false;

	/*
	 * 실제 보이는 낙하물 Mesh.
	 */
	HazardMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HazardMesh"));
	HazardMesh->SetupAttachment(GetRootComponent());
	HazardMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HazardMesh->SetRelativeLocation(FVector::ZeroVector);
	HazardMesh->SetRelativeRotation(FRotator::ZeroRotator);
	HazardMesh->SetRelativeScale3D(FVector(3.f, 3.f, 0.5f));
	//그림자 없애기
	HazardMesh->SetCastShadow(false);

	WarningDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("WarningDecal"));
	WarningDecal->SetupAttachment(GetRootComponent());
	WarningDecal->DecalSize = WarningDecalSize;
	WarningDecal->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
	WarningDecal->SetHiddenInGame(true);

	/*
	 * 피격 판정용 박스.
	 * 실제 충돌 판정은 서버에서만 의미 있게 사용한다.
	 */
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
	WarningDecal->SetHiddenInGame(false);
	DamageBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	/*
	 * 중요:
	 * 타이머와 상태 변화는 서버에서만 실행한다.
	 * 클라이언트에서도 타이머가 돌면 서버 위치와 클라 위치가 서로 다르게 움직일 수 있다.
	 */
	if (HasAuthority() == false)
	{
		return;
	}

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

	/*
	 * 중요:
	 * 낙하물 이동 계산은 서버에서만 한다.
	 * 클라이언트는 서버에서 복제된 위치를 받기만 한다.
	 */
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

		/*
		 * 두 번째 인자 true는 Sweep.
		 * 즉, 이동 중 충돌 체크를 하면서 이동한다.
		 */
		AddActorWorldOffset(DeltaLocation, true);

		if (GetActorLocation().Z <= GroundLocation.Z + GroundCheckDistance)
		{
			FinishImpact();
		}
	}
}

void AGS_FallingHazard::SetTargetActor(AActor* InTargetActor)
{
	/*
	 * 타겟 설정은 서버에서만 의미 있다.
	 * 클라이언트는 이동 계산을 하지 않기 때문.
	 */
	if (HasAuthority() == false)
	{
		return;
	}

	TargetActor = InTargetActor;
}

void AGS_FallingHazard::UpdateTrackingLocation()
{
	if (IsValid(TargetActor) == false)
	{
		return;
	}

	/*
	 * 타겟의 현재 속도를 기준으로 진행 방향을 구한다.
	 */
	FVector Direction = TargetActor->GetVelocity();
	Direction.Z = 0.f;

	/*
	 * 타겟이 멈춰 있으면 속도 방향이 없으므로,
	 * 타겟이 바라보는 앞 방향을 대신 사용한다.
	 */
	if (Direction.IsNearlyZero())
	{
		Direction = TargetActor->GetActorForwardVector();
		Direction.Z = 0.f;
	}

	Direction = Direction.GetSafeNormal();

	const FVector TargetLocation = TargetActor->GetActorLocation();

	/*
	 * 플레이어 위치보다 ForwardDistance만큼 앞을 노린다.
	 */
	const FVector PredictLocation = TargetLocation + Direction * ForwardDistance;

	FVector FoundGroundLocation;
	if (FindGroundLocation(PredictLocation, FoundGroundLocation) == true)
	{
		GroundLocation = FoundGroundLocation;

		// 실제 낙하물은 바닥보다 HeightOffset만큼 위에 둔다.
		SetActorLocation(GroundLocation + FVector(0.f, 0.f, HeightOffset));

		// 데칼은 바닥 위치에 둔다.
		// 살짝 위로 올려서 바닥과 겹침 문제를 줄인다.
		WarningDecal->SetWorldLocation(GroundLocation + FVector(0.f, 0.f, WarningDecalZOffset));

		// 데칼이 항상 아래로 투영되게 유지한다.
		WarningDecal->SetWorldRotation(FRotator(-90.f, 0.f, 0.f));
	}
	else
	{
		SetActorLocation(PredictLocation + FVector(0.f, 0.f, HeightOffset));

		// 바닥을 못 찾았을 때는 예측 위치 근처에 임시로 둔다.
		WarningDecal->SetWorldLocation(PredictLocation);
		WarningDecal->SetWorldRotation(FRotator(-90.f, 0.f, 0.f));
	}

	/*
	 * 위치가 크게 바뀌는 추적 액터라서 즉시 네트워크 업데이트를 요청한다.
	 * 필수는 아니지만 반응이 더 잘 보일 수 있다.
	 */
	ForceNetUpdate();
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

	/*
	 * ECC_Visibility를 막는 바닥/테이블을 찾는다.
	 * 만약 바닥을 못 찾으면 해당 메시의 Collision Response에서 Visibility가 Block인지 확인해야 한다.
	 */
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
	if (HasAuthority() == false)
	{
		return;
	}

	if (State != EGSFallingHazardState::Tracking)
	{
		return;
	}

	State = EGSFallingHazardState::Warning;

	/*
	 * 이 순간의 위치를 낙하 위치로 고정한다.
	 * 이후 WarningTime 동안은 더 이상 플레이어를 추적하지 않는다.
	 */
	FixedDropLocation = GetActorLocation();

	WarningDecal->SetHiddenInGame(false);
	WarningDecal->SetWorldLocation(GroundLocation + FVector(0.f, 0.f, WarningDecalZOffset));
	WarningDecal->SetWorldRotation(FRotator(-90.f, 0.f, 0.f));

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

	// 이제 경고 시간이 끝났으니 데칼은 숨긴다.
	WarningDecal->SetHiddenInGame(true);
	/*
	 * 떨어지는 동안에만 피격 판정을 켠다.
	 */
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
	//충돌 후 데칼 제거
	WarningDecal->SetHiddenInGame(true);

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
	/*
	 * 피격 판정도 서버에서만 처리한다.
	 * 클라이언트에서 맞았다고 판단하면 실제 서버 판정과 달라질 수 있다.
	 */
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

	/*
	 * 나중에 GAS 담당자가 여기서 GameplayEffect를 적용하면 된다.
	 * 예:
	 * - 스턴
	 * - 음식 손실
	 * - HP 감소
	 * - 넉백
	 */
}

void AGS_FallingHazard::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	/*
	 * State를 클라이언트에도 알려준다.
	 * 지금은 주로 디버깅/상태 확인용이고,
	 * 실제 이동과 충돌 처리는 서버에서만 한다.
	 */
	DOREPLIFETIME(AGS_FallingHazard, State);
}