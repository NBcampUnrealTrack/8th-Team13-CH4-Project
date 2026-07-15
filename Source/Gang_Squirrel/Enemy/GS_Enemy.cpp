#include "GS_Enemy.h"

#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Gang_Squirrel/GAS/GA/Attack/Enemy/GA_EnemyAttack.h"
#include "Gang_Squirrel/GAS/GA/Death/GA_EnemyDeath.h"
#include "Gang_Squirrel/GAS/AttributeSet/GS_PlayerAttributeSet.h"
#include "Gang_Squirrel/GAS/Tags/GS_GamePlayTag.h"
#include "Gang_Squirrel/UI/Stat_Widget/GS_HPCountWidget.h"
#include "Net/UnrealNetwork.h"


AGS_Enemy::AGS_Enemy()
{
	PrimaryActorTick.bCanEverTick = true;
	
#pragma region InitGASSystem
	//Create ASC
	EnemyAbilitySystemComp = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	EnemyAbilitySystemComp->SetIsReplicated(true);
	EnemyAbilitySystemComp->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	
	//Create AttributeSet
	EnemyAttributeSet = CreateDefaultSubobject<UGS_PlayerAttributeSet>(TEXT("AttributeSet"));
#pragma endregion 
	
#pragma region CombatComponent
	//Create CombatComponent
	SphereComp_LeftHand = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent_LeftHand"));
	SphereComp_LeftHand->SetupAttachment(GetMesh(), FName("L_Hand"));
	
	SphereComp_RightHand = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent_RightHand"));
	SphereComp_RightHand->SetupAttachment(GetMesh(), FName("R_Hand"));
#pragma endregion
	
#pragma region AnimNotify
	// for AnimNotifyTick Func
	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
#pragma endregion 
	
#pragma region MovementSettings
	// Rotation Settings
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	bUseControllerRotationYaw = false;
#pragma endregion 
	
#pragma region EyeHeightSettings
	BaseEyeHeight = 64.f * GetActorScale3D().Z;
	CrouchedEyeHeight = 64.f * GetActorScale3D().Z;
#pragma endregion 
	
#pragma region HPBarWidget
	HPBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HPBarWidget"));
	HPBarWidget->SetupAttachment(GetMesh());
	HPBarWidget->SetRelativeLocation(FVector(0.f,0.f,120.f));
	HPBarWidget->SetWidgetSpace(EWidgetSpace::World);
#pragma endregion 
	
	
}

void AGS_Enemy::BeginPlay()
{
	Super::BeginPlay();
	
	DefaultMeshRelativeLocation = GetMesh()->GetRelativeLocation();
	DefaultMeshRelativeRotation = GetMesh()->GetRelativeRotation();
	
	HomeLocation = GetActorLocation();
	
	if (const FGS_EnemyDataTable* Row = EnemyDataRow.GetRow<FGS_EnemyDataTable>(TEXT("GS_Enemy:BeginPlay")))
	{
		CachedEnemyData = *Row;
	}
	
	if (HasAuthority())
	{
		EnemyAbilitySystemComp->InitAbilityActorInfo(this,this);
		EnemyAbilitySystemComp->AddLooseGameplayTag(TeamTag::TAG_Team_Enemy);
		
		EnemyAttributeSet->InitHealth(CachedEnemyData.Health);
		EnemyAttributeSet->InitMaxHealth(CachedEnemyData.MaxHealth);
		EnemyAttributeSet->InitMoveSpeed(CachedEnemyData.MoveSpeed);
		
		EnemyAbilitySystemComp->GiveAbility(FGameplayAbilitySpec(GA_Attack,1));
		EnemyAbilitySystemComp->GiveAbility(FGameplayAbilitySpec(GA_Death,1));
		
		for (const TSubclassOf<UGameplayAbility>& ExtraAbility : CachedEnemyData.GrantedAbilities)
		{
			if (ExtraAbility)
			{
				EnemyAbilitySystemComp->GiveAbility(FGameplayAbilitySpec(ExtraAbility,1));
			}
		}
	}
	
	EnemyAbilitySystemComp->RegisterGameplayTagEvent(StateTag::TAG_State_Dead,EGameplayTagEventType::NewOrRemoved).AddUObject(this,&AGS_Enemy::OnDeathStateTagChanged);
	
	// HPBar Settings
	EnemyAbilitySystemComp->GetGameplayAttributeValueChangeDelegate(UGS_PlayerAttributeSet::GetHealthAttribute()).AddUObject(this,&AGS_Enemy::OnHealthAttributeChanged);
	EnemyAbilitySystemComp->GetGameplayAttributeValueChangeDelegate(UGS_PlayerAttributeSet::GetMaxHealthAttribute()).AddUObject(this,&AGS_Enemy::OnHealthAttributeChanged);
	
	RefreshHPBar();
}

void AGS_Enemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	FVector FaceLocation;
	bool bHasFaceTarget = false;
	
	if (RotationTarget)
	{
		FaceLocation = RotationTarget->GetActorLocation();
		bHasFaceTarget = true;
	}
	else if (bRotationTargetIsLocation)
	{
		FaceLocation = RotationTargetLocation;
		bHasFaceTarget = true;
	}
	
	if (bHasFaceTarget)
	{
		const FVector ToTarget = (FaceLocation - GetActorLocation()).GetSafeNormal2D();
		const FRotator DesireRotation = ToTarget.Rotation();
		SetActorRotation(FMath::RInterpTo(GetActorRotation(), DesireRotation, DeltaTime, RotationInterpSpeed));
	}
	
	if (HPBarWidget)
	{
		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			FVector CameraLocation;
			FRotator CameraRotation;
			PC->GetPlayerViewPoint(CameraLocation,CameraRotation);
			
			FRotator LookAtRotation = (CameraLocation - HPBarWidget->GetComponentLocation()).Rotation();
			LookAtRotation.Pitch = 0.f;
			LookAtRotation.Roll = 0.f;
			HPBarWidget->SetWorldRotation(LookAtRotation);
		}
	}
}

UAbilitySystemComponent* AGS_Enemy::GetAbilitySystemComponent() const
{
	return EnemyAbilitySystemComp;
}

void AGS_Enemy::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AGS_Enemy,RotationTarget);
	DOREPLIFETIME(AGS_Enemy,RotationInterpSpeed);
	DOREPLIFETIME(AGS_Enemy,bRotationTargetIsLocation);
	DOREPLIFETIME(AGS_Enemy,RotationTargetLocation);
}

void AGS_Enemy::SetRotationTarget(AActor* NewTarget, float NewInterpSpeed)
{
	RotationTarget = NewTarget;
	bRotationTargetIsLocation = false;
	RotationInterpSpeed = NewInterpSpeed;
}

void AGS_Enemy::SetRotationTarget(const FVector& NewLocation, float NewInterpSpeed)
{
	RotationTarget = nullptr;
	bRotationTargetIsLocation = true;
	RotationTargetLocation = NewLocation;
	RotationInterpSpeed = NewInterpSpeed;
}

void AGS_Enemy::OnDeathStateTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	SetActorEnableCollision(NewCount <= 0);
}

void AGS_Enemy::SetKillerPlayerState(AGS_PlayerState* InKiller)
{
	KillerPlayerState = InKiller;
}

void AGS_Enemy::NetMultiCast_FreezeDeathPose_Implementation()
{
	GetMesh()->bPauseAnims = true;
}

void AGS_Enemy::RefreshHPBar()
{
	if (UGS_HPCountWidget* HPWidget = Cast<UGS_HPCountWidget>(HPBarWidget->GetWidget()))
	{
		HPWidget->SetHealth(FMath::RoundToInt(EnemyAttributeSet->GetHealth()),FMath::RoundToInt(EnemyAttributeSet->GetMaxHealth()));
	}
}

void AGS_Enemy::OnHealthAttributeChanged(const FOnAttributeChangeData& Data)
{
	RefreshHPBar();
}

void AGS_Enemy::SetupUpperBodyRagdoll()
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}
	
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!MeshComp)
	{
		return;
	}
	
	MeshComp->SetCollisionProfileName(RagdollCollisionProfile);
	MeshComp->RecreatePhysicsState();
	MeshComp->SetAllBodiesBelowSimulatePhysics(RagdollStartBone,true,true);
}

void AGS_Enemy::NetMulticast_ApplyRagdollImpulse_Implementation(FVector Impulse, FName BoneName)
{
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->AddImpulseAtLocation(Impulse, MeshComp->GetBoneLocation(BoneName),BoneName);
	}
}

void AGS_Enemy::NetMulticast_SetFullRagdollEnable_Implementation(bool bEnable)
{
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!MeshComp)
	{
		return;
	}
	
	if (bEnable)
	{
		MeshComp->SetAllBodiesSimulatePhysics(true);
		MeshComp->WakeAllRigidBodies();
	}
	else
	{
		MeshComp->SetAllBodiesSimulatePhysics(false);
		MeshComp->SetRelativeLocationAndRotation(DefaultMeshRelativeLocation,DefaultMeshRelativeRotation);
		SetupUpperBodyRagdoll();
	}
}

void AGS_Enemy::Applyknockdown(FVector Impulse, FName BoneName, float Duration)
{
	if (!HasAuthority())
	{
		return;
	}
	
	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		if (UBrainComponent* BrainComp = AIController->GetBrainComponent())
		{
			BrainComp->PauseLogic(TEXT("Enemy Knockdown"));
		}
	}
	
	NetMulticast_SetFullRagdollEnable(true);
	NetMulticast_ApplyRagdollImpulse(Impulse,BoneName);
	
	GetWorld()->GetTimerManager().SetTimer(KnockdownRecoveryTimerHandle,this,&AGS_Enemy::RecoverFromKnockdown,Duration,false);
}

void AGS_Enemy::RecoverFromKnockdown()
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		if (ASC->HasMatchingGameplayTag(StateTag::TAG_State_Dead))
		{
			return;
		}
	}
	
	RepositionCapsuleToRagdoll();
	NetMulticast_SetFullRagdollEnable(false);
	
	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		if (UBrainComponent* BrainComp = AIController->GetBrainComponent())
		{
			BrainComp->ResumeLogic(TEXT("Enemy Knockdown Recovered"));
		}
	}
}

void AGS_Enemy::RepositionCapsuleToRagdoll()
{
	USkeletalMeshComponent* MeshComp = GetMesh();
	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	
	if (!MeshComp || !CapsuleComp)
	{
		return;
	}
	
	const FVector PelvisLocation = MeshComp->GetBoneLocation(TEXT("Pelvis"));
	
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	
	FHitResult FloorHit;
	FVector NewLocation = PelvisLocation;
	
	if (GetWorld()->LineTraceSingleByChannel(FloorHit, PelvisLocation, PelvisLocation - FVector(0.f,0.f,500.f),ECC_Visibility,QueryParams))
	{
		NewLocation = FloorHit.ImpactPoint + FVector(0.f,0.f,CapsuleComp->GetScaledCapsuleHalfHeight());
	}
	
	const FRotator NewRotation(0.f, MeshComp->GetSocketRotation(TEXT("Pelvis")).Yaw, 0.f);
	
	SetActorLocationAndRotation(NewLocation,NewRotation,false,nullptr,ETeleportType::TeleportPhysics);
}