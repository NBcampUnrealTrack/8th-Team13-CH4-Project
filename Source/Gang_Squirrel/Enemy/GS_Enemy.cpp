#include "GS_Enemy.h"

#include "AbilitySystemComponent.h"
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
	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickMontagesAndRefreshBonesWhenPlayingMontages;
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