#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GS_FallingHazard.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class UDecalComponent;

/*
 * 낙하물의 현재 상태.
 *
 * Tracking : 플레이어 위치를 추적하는 상태
 * Warning  : 위치가 고정되고, 떨어지기 전 잠깐 대기하는 상태
 * Falling  : 실제로 아래로 떨어지는 상태
 * Impact   : 바닥에 닿은 뒤 처리 상태
 */
UENUM(BlueprintType)
enum class EGSFallingHazardState : uint8
{
	Tracking,
	Warning,
	Falling,
	Impact
};

/*
 * 기존에는 AActor였지만, 서버 이동 동기화를 더 안정적으로 쓰기 위해 ACharacter로 변경.
 *
 * ACharacter는 기본적으로 CapsuleComponent와 CharacterMovementComponent를 가지고 있고,
 * SetReplicateMovement(true)를 켜면 서버에서 움직인 위치가 클라이언트로 복제된다.
 */
UCLASS()
class GANG_SQUIRREL_API AGS_FallingHazard : public ACharacter
{
	GENERATED_BODY()

public:
	AGS_FallingHazard();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	/*
	 * 매니저가 이 낙하물이 추적할 타겟을 넣어주는 함수.
	 * 서버에서 Spawn 후 호출된다.
	 */
	void SetTargetActor(AActor* InTargetActor);

protected:
	/*
	 * 실제 보이는 낙하물 메시.
	 * ACharacter는 이미 CapsuleComponent를 Root로 가지고 있기 때문에
	 * 별도의 SceneRoot를 만들지 않고 CapsuleComponent 아래에 붙인다.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UStaticMeshComponent> HazardMesh;

	/*
	 * 플레이어와 겹쳤는지 확인하는 피격 박스.
	 * 실제 판정은 서버에서만 처리한다.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UBoxComponent> DamageBox;

	//바닥에 표시될 데칼
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UDecalComponent> WarningDecal;


protected:
	/*
	 * 플레이어의 이동 방향 기준으로 얼마나 앞을 노릴지.
	 * 0이면 플레이어 머리 위를 따라다닌다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling Hazard|Tracking")
	float ForwardDistance = 500.f;

	/*
	 * 바닥 위치 기준으로 낙하물이 얼마나 높은 곳에 떠 있을지.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling Hazard|Tracking")
	float HeightOffset = 1000.f;

	/*
	 * 몇 초 동안 플레이어를 추적할지.
	 * 이 시간이 끝나면 위치가 고정된다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling Hazard|Time")
	float TrackingTime = 3.f;

	/*
	 * 위치가 고정된 뒤, 떨어지기 전까지 기다리는 시간.
	 * WarningMesh가 없어도 실제 물체/그림자가 경고 역할을 한다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling Hazard|Time")
	float WarningTime = 2.f;

	/*
	 * 낙하 속도.
	 * Tick에서 DeltaTime과 곱해서 매 프레임 아래로 이동시킨다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling Hazard|Falling")
	float FallSpeed = 1500.f;

	/*
	 * 바닥에 닿은 뒤 몇 초 후 삭제할지.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling Hazard|Falling")
	float ImpactDestroyDelay = 0.3f;

	/*
	 * 바닥을 찾기 위한 LineTrace 시작 높이.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling Hazard|Trace")
	float GroundTraceUpOffset = 1000.f;

	/*
	 * 바닥을 찾기 위한 LineTrace 아래 방향 길이.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling Hazard|Trace")
	float GroundTraceDownOffset = 3000.f;

	/*
	 * 낙하물이 바닥에 얼마나 가까워지면 Impact 처리할지.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling Hazard|Trace")
	float GroundCheckDistance = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling Hazard|Warning")
	FVector WarningDecalSize = FVector(128.f, 300.f, 300.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling Hazard|Warning")
	float WarningDecalZOffset = 5.f;

	protected:
		// 데칼이 바닥에 투영되는 깊이.
		// 너무 작으면 바닥에 제대로 안 보일 수 있다.
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling Hazard|Warning")
		float WarningDecalProjectionDepth = 300.f;

		// 낙하 시작 시 데칼 크기.
		// 위험 표시가 처음에는 작게 보이게 하는 값.
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling Hazard|Warning")
		float WarningDecalStartSize = 120.f;

		// 낙하물이 바닥에 가까워졌을 때 데칼 최종 크기.
		// 값이 클수록 가까이 오는 느낌이 강해진다.
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling Hazard|Warning")
		float WarningDecalEndSize = 420.f;


	
protected:
	/*
	 * 현재 낙하물 상태.
	 * Replicated로 둔 이유:
	 * 서버에서 상태가 바뀐 것을 클라이언트도 알 수 있게 하기 위해서.
	 * 다만 실제 이동/판정은 서버에서만 한다.
	 */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Falling Hazard")
	EGSFallingHazardState State = EGSFallingHazardState::Tracking;

	/*
	 * 추적 대상.
	 * 서버에서만 유효하게 사용한다.
	 */
	UPROPERTY()
	TObjectPtr<AActor> TargetActor;

	/*
	 * LineTrace로 찾은 바닥 위치.
	 */
	UPROPERTY()
	FVector GroundLocation = FVector::ZeroVector;

	/*
	 * 추적이 끝났을 때 고정된 낙하 시작 위치.
	 */
	UPROPERTY()
	FVector FixedDropLocation = FVector::ZeroVector;

	/*
	 * 같은 액터를 여러 번 맞추지 않기 위한 기록.
	 */
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

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};