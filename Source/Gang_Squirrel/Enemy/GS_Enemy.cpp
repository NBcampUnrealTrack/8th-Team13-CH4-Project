#include "GS_Enemy.h"

#include "AbilitySystemComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Gang_Squirrel/GAS/GA/Attack/Enemy/GA_EnemyAttack.h"
#include "Gang_Squirrel/GAS/GA/Death/GA_EnemyDeath.h"
#include "Gang_Squirrel/GAS/AttributeSet/GS_PlayerAttributeSet.h"
#include "Gang_Squirrel/GAS/Tags/GS_GamePlayTag.h"
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
	
	// for AnimNotifyTick Func
	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickMontagesAndRefreshBonesWhenPlayingMontages;
	// Rotation Settings
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	bUseControllerRotationYaw = false;
}

void AGS_Enemy::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		EnemyAbilitySystemComp->InitAbilityActorInfo(this,this);
		EnemyAbilitySystemComp->AddLooseGameplayTag(TeamTag::TAG_Team_Enemy);
		EnemyAbilitySystemComp->GiveAbility(FGameplayAbilitySpec(GA_Attack,1));
		EnemyAbilitySystemComp->GiveAbility(FGameplayAbilitySpec(GA_Death,1));
	}
	
	EnemyAbilitySystemComp->RegisterGameplayTagEvent(StateTag::TAG_State_Dead,EGameplayTagEventType::NewOrRemoved).AddUObject(this,&AGS_Enemy::OnDeathStateTagChanged);
}

void AGS_Enemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (RotationTarget)
	{
		const FVector ToTarget = (RotationTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
		const FRotator DesiredRotation = ToTarget.Rotation();

		SetActorRotation(FMath::RInterpTo(GetActorRotation(), DesiredRotation, DeltaTime, RotationInterpSpeed));
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
}

USphereComponent* AGS_Enemy::GetCombatCollision(EHandCombatType HandType) const
{
	USphereComponent* CombatHandComp = nullptr;
	
	if (HandType == EHandCombatType::RightCombatHand)
	{
		CombatHandComp = SphereComp_RightHand;
	}
	else
	{
		CombatHandComp = SphereComp_LeftHand;
	}
	
	return CombatHandComp;
}

void AGS_Enemy::SetRotationTarget(AActor* NewTarget, float NewInterpSpeed)
{
	RotationTarget = NewTarget;
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