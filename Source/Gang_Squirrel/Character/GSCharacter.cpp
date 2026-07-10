// Fill out your copyright notice in the Description page of Project Settings.


#include "GSCharacter.h"

#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Gang_Squirrel/Player/GS_PlayerState.h"
#include "Components/SphereComponent.h"
#include "Gang_Squirrel/GAS/AttributeSet/GS_PlayerAttributeSet.h"
#include "Gang_Squirrel/GAS/GA/Attack/GA_Attack.h"
#include "Gang_Squirrel/GAS/GA/Roll/GA_Roll.h"
#include "Gang_Squirrel/GAS/GA/Sprint/GA_Sprint.h"
#include "Gang_Squirrel/GAS/GA/Death/GA_PlayerDeath.h"
#include "Gang_Squirrel/GAS/Tags/GS_GamePlayTag.h"
#include "Components/WidgetComponent.h"
#include "Gang_Squirrel/Food/GSFoodBase.h"
#include "Gang_Squirrel/UI/GS_StaminaBarWidget.h"
#include "Gang_Squirrel/UI/GSPlayerNameTag.h"
#include "Gang_Squirrel/Gang_Squirrel.h"
#include "Gang_Squirrel/Food/GSCheekWidget.h"
#include "Net/UnrealNetwork.h"

AGSCharacter::AGSCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCharacterMovement()->NetworkSmoothingMode = ENetworkSmoothingMode::Exponential;//smooth

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;
	
	bReplicates = true;
	SetReplicatingMovement(true);

	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 400.f, 0.0f); //Rotation 400/s

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetRootComponent());
	SpringArm->TargetArmLength = 450.f;
	SpringArm->SetRelativeRotation(FRotator(-18.f, 0.f, 0.f));
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->bDoCollisionTest = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
	Camera->bUsePawnControlRotation = false;

	leftHandCollision = CreateDefaultSubobject<USphereComponent>(TEXT("leftHandCollision"));
	leftHandCollision->SetupAttachment(GetMesh(), TEXT("L_Hand"));

	rightHandCollision = CreateDefaultSubobject<USphereComponent>(TEXT("rightHandCollision"));
	rightHandCollision->SetupAttachment(GetMesh(), TEXT("R_Hand"));

	//Nickname Widget Component
	PlayerNameTagWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PlayerNameTagWidget"));
	PlayerNameTagWidget->SetupAttachment(GetMesh());
	PlayerNameTagWidget->SetRelativeLocation(FVector(0.0f, 0.0f, 120.0f)); // 머리 위
	PlayerNameTagWidget->SetWidgetSpace(EWidgetSpace::Screen); // 항상 카메라를 향함
	
	//Stamina Widget Component
	StaminaBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("StaminaBarWidget"));
	StaminaBarWidget->SetupAttachment(GetRootComponent());
	StaminaBarWidget->SetRelativeLocation(FVector(0.f, 50.f, -20.f));
	StaminaBarWidget->SetWidgetSpace(EWidgetSpace::Screen);
	StaminaBarWidget->SetDrawSize(FVector2D(32.f, 160.f));
	StaminaBarWidget->SetOnlyOwnerSee(true);
	StaminaBarWidget->SetVisibility(false);
	StaminaBarWidget->SetHiddenInGame(true);
	StaminaBarWidget->SetUsingAbsoluteLocation(true);
	// for AnimNotifyTick Func
	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickMontagesAndRefreshBonesWhenPlayingMontages;
}

void AGSCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsRolling)
	{
		AddMovementInput(RollingDirection, 1.0f);
	}

	UpdateStaminaBarWorldLocation();
}

void AGSCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocallyControlled())
	{
		APlayerController* PC = Cast<APlayerController>(GetController());
		if (IsValid(PC))
		{
			UEnhancedInputLocalPlayerSubsystem* EILPS =
				ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());

			EILPS->AddMappingContext(IMC, 0);
		}

		UpdateStaminaBarWorldLocation();

	}
	
	AGS_PlayerState* PS = GetPlayerState<AGS_PlayerState>();
	if (IsValid(PS))
	{
		
		if (PS->OnPlayerNameChanged.IsAlreadyBound(this, &ThisClass::UpdateNameTag) == false)
		{
			PS->OnPlayerNameChanged.AddDynamic(this, &ThisClass::UpdateNameTag);
		}


		//If controller already has nickname.
		if (PS->PlayerNickname.IsEmpty() == false)
		{
			UpdateNameTag(PS->PlayerNickname);
		}
	}

	BindMovementSpeedDelegates();

	if (IsLocallyControlled())
	{
		BindStaminaDelegates();
	}
	
	//Cheek  Widget
	if (IsLocallyControlled() && CheekWidgetClass)
	{
		APlayerController* PC = Cast<APlayerController>(GetController());
		if (PC)
		{
			CheekWidgetUIInstance = CreateWidget<UGSCheekWidget>(PC, CheekWidgetClass);
			
			if (CheekWidgetUIInstance)
			{
				CheekWidgetUIInstance->InitCheekWidget();
				
				CheekWidgetUIInstance->AddToViewport();
			}
		}
	}
}

void AGSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (IsValid(EIC))
	{
		EIC->BindAction(Move, ETriggerEvent::Triggered, this, &ThisClass::IAMove);
		EIC->BindAction(Look, ETriggerEvent::Triggered, this, &ThisClass::IALook);
		EIC->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		EIC->BindAction(Interact, ETriggerEvent::Started, this, &ThisClass::IAInteract);
		EIC->BindAction(Interact, ETriggerEvent::Canceled, this, &ThisClass::IAStopInteract);
		EIC->BindAction(Interact, ETriggerEvent::Completed, this, &ThisClass::IAStopInteract);
		EIC->BindAction(Attack, ETriggerEvent::Started, this, &ThisClass::IAAttack);
		EIC->BindAction(Sprint, ETriggerEvent::Started, this, &ThisClass::IAStartSprint);
		EIC->BindAction(Sprint, ETriggerEvent::Completed, this, &ThisClass::IAEndSprint);
		EIC->BindAction(Rolling, ETriggerEvent::Started, this, &ThisClass::IARolling);
	}
}

void AGSCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGSCharacter, CurrentCheekSize);
	DOREPLIFETIME(AGSCharacter, MaxCheekSize);
}

void AGSCharacter::IAMove(const FInputActionValue& InValue)
{
	if (false == IsValid(Controller))
	{
		return;
	}

	const FVector2D InMovementVector = InValue.Get<FVector2D>(); //XY Vector Value(-1 ~ 1)

	const FRotator ControlRotation = Controller->GetControlRotation(); //use only Yaw Value
	const FRotator ControlYawRotation(0.0f, ControlRotation.Yaw, 0.0f);

	const FVector ForwardDirection = FRotationMatrix(ControlYawRotation).GetUnitAxis(EAxis::X); //ForwardVector
	const FVector RightDirection = FRotationMatrix(ControlYawRotation).GetUnitAxis(EAxis::Y); //RightVector

	AddMovementInput(ForwardDirection, InMovementVector.X);//W S
	AddMovementInput(RightDirection, InMovementVector.Y);//A D
}

void AGSCharacter::IALook(const FInputActionValue& InValue)
{
	if (IsValid(Controller) == false)
	{
		return;
	}

	const FVector2D InLookVector = InValue.Get<FVector2D>();

	//Change controller rotation
	AddControllerYawInput(InLookVector.X);
	AddControllerPitchInput(InLookVector.Y);
}

void AGSCharacter::IAInteract(const FInputActionValue& InValue)
{
	UE_LOG(LogTemp, Log, TEXT("Interact!"));
	
	if (CurrentCheekSize >= MaxCheekSize) return;
	
	if (bIsEating) return;

	bIsEating = true;

	// EatingAnimation
	if (AM_Eat)
	{
		PlayAnimMontage(AM_Eat);
	}

	Server_SetEating(true);
}

void AGSCharacter::IAStopInteract(const FInputActionValue& InValue)
{
	UE_LOG(LogTemp, Log, TEXT("StopInteract!"));


	if (!bIsEating) return;

	bIsEating = false;

	if (AM_Eat)
	{
		StopAnimMontage(AM_Eat);
	}

	Server_SetEating(false);
}

void AGSCharacter::Server_SetEating_Implementation(bool bEating)
{
	UE_LOG(LogTemp, Log, TEXT("SetEating!"));

	bIsEating = bEating;

	Multicast_SetEatingAnimation(bEating);
}

void AGSCharacter::InflateCheeks(float Value)
{
	if (HasAuthority())
	{
		Multicast_InflateCheeks(Value);
	}
}

void AGSCharacter::Multicast_SetEatingAnimation_Implementation(bool bEating)
{
	if (IsLocallyControlled())
	{
		return;
	}

	bIsEating = bEating;

	if (bEating)
	{
		if (AM_Eat)
		{
			PlayAnimMontage(AM_Eat);
		}
	}
	else
	{
		if (AM_Eat)
		{
			StopAnimMontage(AM_Eat);
		}
	}
}

void AGSCharacter::OnRep_CheekSize()
{
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (IsValid(MeshComp) && MaxCheekSize > 0.f)
	{
		MeshComp->SetMorphTarget(FName("CheeksSize"), CurrentCheekSize / MaxCheekSize);
	}
}

void AGSCharacter::Multicast_InflateCheeks_Implementation(float Value)
{
	USkeletalMeshComponent* MeshComp = GetMesh();
	
	float TempValue = CurrentCheekSize + Value;
	if (MaxCheekSize < TempValue)
	{
		CurrentCheekSize = MaxCheekSize;
	}
	else
	{
		CurrentCheekSize += Value;
	}
	
	
	float ResultValue = CurrentCheekSize / MaxCheekSize;
	
	if (CheekWidgetUIInstance)
	{
		CheekWidgetUIInstance->SetProgressValue(ResultValue);
	}
	
	if (MeshComp)
	{
		MeshComp->SetMorphTarget(FName("CheeksSize"), ResultValue);
	}
}

void AGSCharacter::ResetCheekSize()
{
	CurrentCheekSize = 0.f;
	
	Multicast_InflateCheeks(0.f);
	
	UE_LOG(LogTemp, Error, TEXT("ResetCheekSize!"));
}

void AGSCharacter::AddMaxCheekSize(float Value)
{
	MaxCheekSize += Value;
	
	Multicast_InflateCheeks(0.f);
}

void AGSCharacter::IAAttack(const FInputActionValue& InValue)
{
	UE_LOG(LogTemp, Log, TEXT("Attack!"));
	
	AGS_PlayerState* PS = GetPlayerState<AGS_PlayerState>();
	if (PS)
	{
		PS->GetAbilitySystemComponent()->TryActivateAbilitiesByTag(FGameplayTagContainer(AbilityTag::TAG_Ability_Attack));
	}
}

void AGSCharacter::IAStartSprint(const FInputActionValue& InValue)
{
	UE_LOG(LogTemp, Log, TEXT("Start Sprint!"));

	AGS_PlayerState* PS = GetPlayerState<AGS_PlayerState>();
	if (PS)
	{
		PS->GetAbilitySystemComponent()->TryActivateAbilitiesByTag(
			FGameplayTagContainer(AbilityTag::TAG_Ability_Sprint)
		);
	}
}
void AGSCharacter::IAEndSprint(const FInputActionValue& InValue)
{
	UE_LOG(LogTemp, Log, TEXT("Complete Sprint!"));

	AGS_PlayerState* PS = GetPlayerState<AGS_PlayerState>();
	if (PS)
	{
		FGameplayTagContainer SprintTagContainer;
		SprintTagContainer.AddTag(AbilityTag::TAG_Ability_Sprint);

		PS->GetAbilitySystemComponent()->CancelAbilities(&SprintTagContainer);
	}
}

void AGSCharacter::IARolling(const FInputActionValue& InValue)
{
	UE_LOG(LogTemp, Log, TEXT("Rolling!"));

	AGS_PlayerState* PS = GetPlayerState<AGS_PlayerState>();
	if (PS)
	{
		PS->GetAbilitySystemComponent()->TryActivateAbilitiesByTag(FGameplayTagContainer(AbilityTag::TAG_Ability_Roll));
	}
}

void AGSCharacter::SetSprinting(bool bNewSprinting)
{
	bIsSprinting = bNewSprinting;

	UpdateMaxWalkSpeedFromAttribute();
}

void AGSCharacter::StartSprintFromAbility()
{
	SetSprinting(true);
}

void AGSCharacter::StopSprintFromAbility()
{
	SetSprinting(false);
}

void AGSCharacter::UpdateNameTag(const FString& Newname)
{
	UGSPlayerNameTag* NameTag = Cast<UGSPlayerNameTag>(PlayerNameTagWidget->GetWidget());

	if (NameTag)
	{
		NameTag->SetNickname(Newname);
	}
}

void AGSCharacter::RollFromAbility()
{
	if (bIsRolling)
	{
		return;
	}

	const FVector RollDirection = GetRollingDirection();

	if (HasAuthority())
	{
		StartRolling(RollDirection);
	}
	else
	{
		StartRollingLocal(RollDirection);
		ServerStartRolling(RollDirection);
	}
}

void AGSCharacter::BindMovementSpeedDelegates()
{
	if (bMovementSpeedDelegateBound)
	{
		return;
	}

	AGS_PlayerState* PS = GetPlayerState<AGS_PlayerState>();
	if (PS == nullptr)
	{
		return;
	}

	UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
	if (ASC == nullptr)
	{
		return;
	}

	ASC->GetGameplayAttributeValueChangeDelegate(UGS_PlayerAttributeSet::GetMoveSpeedAttribute())
		.AddUObject(this, &AGSCharacter::OnMoveSpeedChanged);

	ASC->GetGameplayAttributeValueChangeDelegate(UGS_PlayerAttributeSet::GetSlowSpeedMultiplierAttribute())
		.AddUObject(this, &AGSCharacter::OnSlowSpeedMultiplierChanged);

	CachedMoveSpeed = ASC->GetNumericAttribute(UGS_PlayerAttributeSet::GetMoveSpeedAttribute());
	CachedSlowSpeedMultiplier = ASC->GetNumericAttribute(UGS_PlayerAttributeSet::GetSlowSpeedMultiplierAttribute());

	bMovementSpeedDelegateBound = true;

	UpdateMaxWalkSpeedFromAttribute();
}

//Caching MoveSpeed
void AGSCharacter::OnMoveSpeedChanged(const FOnAttributeChangeData& Data)
{
	CachedMoveSpeed = Data.NewValue;
	UpdateMaxWalkSpeedFromAttribute();
}

//Caching SlowSpeedMultiplier
void AGSCharacter::OnSlowSpeedMultiplierChanged(const FOnAttributeChangeData& Data)
{
	CachedSlowSpeedMultiplier = Data.NewValue;
	UpdateMaxWalkSpeedFromAttribute();
}

float AGSCharacter::GetFinalMoveSpeedMultiplier() const
{
	if (bIsRolling)
	{
		return RollSpeedMultiplier;
	}

	if (bIsSprinting)
	{
		return SprintSpeedMultiplier;
	}

	return 1.f;
}

//MaxWalkSpeed
void AGSCharacter::UpdateMaxWalkSpeedFromAttribute()
{
	const float SafeMoveSpeed = FMath::Max(CachedMoveSpeed, 0.f);
	const float SafeSlowMultiplier = FMath::Clamp(CachedSlowSpeedMultiplier, 0.1f, 1.f);

	const float FinalSpeed =
		SafeMoveSpeed *
		SafeSlowMultiplier *
		GetFinalMoveSpeedMultiplier();

	GetCharacterMovement()->MaxWalkSpeed = FinalSpeed;
}

void AGSCharacter::StartRolling(const FVector& InRollingDirection)
{
	if (bIsRolling)
	{
		return;
	}

	bIsRolling = true;

	RollingDirection = InRollingDirection;
	RollingDirection.Z = 0.f;
	RollingDirection.Normalize();

	UpdateMaxWalkSpeedFromAttribute();

	GetWorldTimerManager().ClearTimer(RollingTimerHandle);
	GetWorldTimerManager().SetTimer(
		RollingTimerHandle,
		this,
		&AGSCharacter::FinishRolling,
		RollingDuration,
		false
	);

	MulticastPlayRollMontage();

	UE_LOG(LogTemp, Log, TEXT("Start Rolling"));
}

void AGSCharacter::StartRollingLocal(const FVector& InRollingDirection)
{
	if (bIsRolling)
	{
		return;
	}

	bIsRolling = true;

	RollingDirection = InRollingDirection;
	RollingDirection.Z = 0.f;
	RollingDirection.Normalize();

	UpdateMaxWalkSpeedFromAttribute();

	if (AM_Roll)
	{
		PlayAnimMontage(AM_Roll);
	}

	GetWorldTimerManager().ClearTimer(RollingTimerHandle);
	GetWorldTimerManager().SetTimer(
		RollingTimerHandle,
		this,
		&AGSCharacter::FinishRolling,
		RollingDuration,
		false
	);
}

void AGSCharacter::FinishRolling()
{
	bIsRolling = false;
	RollingDirection = FVector::ZeroVector;

	UpdateMaxWalkSpeedFromAttribute();
}

FVector AGSCharacter::GetRollingDirection() const
{
	FVector Direction = GetLastMovementInputVector();

	if (Direction.IsNearlyZero())
	{
		Direction = GetActorForwardVector();
	}

	Direction.Z = 0.f;
	Direction.Normalize();

	return Direction;
}

void AGSCharacter::MulticastPlayRollMontage_Implementation()
{
	if(IsLocallyControlled())
	{
		return;
	}

	if (AM_Roll == nullptr)
	{
		return;
	}

	PlayAnimMontage(AM_Roll);
}

void AGSCharacter::ServerStartRolling_Implementation(FVector_NetQuantizeNormal InRollingDirection)
{
	if (bIsRolling)
	{
		return;
	}

	StartRolling(InRollingDirection);
}

void AGSCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// Server
	AGS_PlayerState* PS = GetPlayerState<AGS_PlayerState>();
	if (PS)
	{
		// Init ASC
		PS->GetAbilitySystemComponent()->InitAbilityActorInfo(PS,this);
		PS->GetAbilitySystemComponent()->AddLooseGameplayTag(TeamTag::TAG_Team_Player);

		BindMovementSpeedDelegates();
		//Give Ability
		if (!PS->GetAbilitySystemComponent()->FindAbilitySpecFromClass(UGA_Attack::StaticClass()))
		{
			PS->GetAbilitySystemComponent()->GiveAbility(FGameplayAbilitySpec(GA_Attack,1));
		}
		if (!PS->GetAbilitySystemComponent()->FindAbilitySpecFromClass(UGA_PlayerDeath::StaticClass()))
		{
			PS->GetAbilitySystemComponent()->GiveAbility(FGameplayAbilitySpec(GA_Death,1));
		}
		if (!PS->GetAbilitySystemComponent()->FindAbilitySpecFromClass(UGA_Roll::StaticClass()))
		{
			PS->GetAbilitySystemComponent()->GiveAbility(FGameplayAbilitySpec(GA_Roll, 1));
		}
		if (!PS->GetAbilitySystemComponent()->FindAbilitySpecFromClass(UGA_Sprint::StaticClass()))
		{
			PS->GetAbilitySystemComponent()->GiveAbility(FGameplayAbilitySpec(GA_Sprint, 1));
		}
		
		// When State.Dead Tag Was Attached or Detached Call to Func
		PS->GetAbilitySystemComponent()->RegisterGameplayTagEvent(StateTag::TAG_State_Dead, EGameplayTagEventType::NewOrRemoved).AddUObject(this,&AGSCharacter::OnDeathStateTagChanged);
	}
}

void AGSCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	//Client
	AGS_PlayerState* PS = GetPlayerState<AGS_PlayerState>();
	if (PS)
	{
		// Init ASC
		PS->GetAbilitySystemComponent()->InitAbilityActorInfo(PS,this);

		BindMovementSpeedDelegates();

		if (PS->OnPlayerNameChanged.IsAlreadyBound(this, &ThisClass::UpdateNameTag) == false)
		{
			PS->OnPlayerNameChanged.AddDynamic(this, &ThisClass::UpdateNameTag);
		}

		if (PS->PlayerNickname.IsEmpty() == false)
		{
			UpdateNameTag(PS->PlayerNickname);
		}
		
		// When State.Dead Tag Was Attached or Detached Call to Func
		PS->GetAbilitySystemComponent()->RegisterGameplayTagEvent(StateTag::TAG_State_Dead, EGameplayTagEventType::NewOrRemoved).AddUObject(this,&AGSCharacter::OnDeathStateTagChanged);
	
		if (IsLocallyControlled())
		{
			BindStaminaDelegates();
		}
	}
}

UAbilitySystemComponent* AGSCharacter::GetAbilitySystemComponent() const
{
	AGS_PlayerState* PS = GetPlayerState<AGS_PlayerState>();
	return PS ? PS->GetAbilitySystemComponent() : nullptr;
}

void AGSCharacter::Server_NotifyFoodEaten_Implementation(AGSFoodBase* EatenFood)
{
	if (!EatenFood) return;
	
	AGS_PlayerState* PS = GetPlayerState<AGS_PlayerState>();
	if (PS)
	{
		PS->AddScore(EatenFood->Eaten());
	}
}

// GA_Death Callback Func : Temp Logic
void AGSCharacter::OnDeathStateTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	SetActorEnableCollision(NewCount <= 0);
}

// For After GA_Death Frozen Death Animation Func
void AGSCharacter::NetMulticast_SetDeathPoseFrozen_Implementation(bool bFrozen)
{
	GetMesh()->bPauseAnims = bFrozen;
}

//Stamina
void AGSCharacter::BindStaminaDelegates()
{
	if (!IsLocallyControlled())
	{
		return;
	}

	if (bStaminaDelegateBound)
	{
		return;
	}

	AGS_PlayerState* PS = GetPlayerState<AGS_PlayerState>();
	if (PS == nullptr)
	{
		return;
	}

	UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
	if (ASC == nullptr)
	{
		return;
	}

	ASC->GetGameplayAttributeValueChangeDelegate(UGS_PlayerAttributeSet::GetStaminaAttribute())
		.AddUObject(this, &AGSCharacter::OnStaminaChanged);

	ASC->GetGameplayAttributeValueChangeDelegate(UGS_PlayerAttributeSet::GetMaxStaminaAttribute())
		.AddUObject(this, &AGSCharacter::OnMaxStaminaChanged);

	const float CurrentStamina = ASC->GetNumericAttribute(UGS_PlayerAttributeSet::GetStaminaAttribute());
	const float MaxStamina = ASC->GetNumericAttribute(UGS_PlayerAttributeSet::GetMaxStaminaAttribute());

	CachedMaxStamina = MaxStamina;
	UpdateStaminaBar(CurrentStamina, MaxStamina);
	RefreshStaminaBarVisibility(CurrentStamina, MaxStamina);
}

void AGSCharacter::UpdateStaminaBar(float CurrentStamina, float MaxStamina)
{
	if (!IsLocallyControlled())
	{
		return;
	}

	if (MaxStamina <= 0.f)
	{
		return;
	}

	UGS_StaminaBarWidget* StaminaWidget = Cast<UGS_StaminaBarWidget>(StaminaBarWidget->GetUserWidgetObject());
	if (StaminaWidget == nullptr)
	{
		return;
	}

	const float Percent = CurrentStamina / MaxStamina;
	StaminaWidget->SetStaminaPercent(Percent);
}

void AGSCharacter::RefreshStaminaBarVisibility(float CurrentStamina, float MaxStamina)
{
	if (!IsLocallyControlled())
	{
		return;
	}

	if (StaminaBarWidget == nullptr)
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(StaminaBarHideTimerHandle);

	const bool bShouldShow = CurrentStamina < MaxStamina;

	if (bShouldShow)
	{
		StaminaBarWidget->SetHiddenInGame(false);
		StaminaBarWidget->SetVisibility(true);
	}
	else
	{
		GetWorldTimerManager().SetTimer(
			StaminaBarHideTimerHandle,
			this,
			&AGSCharacter::HideStaminaBar,
			0.75f,
			false
		);
	}
}

void AGSCharacter::HideStaminaBar()
{
	if (StaminaBarWidget)
	{
		StaminaBarWidget->SetVisibility(false);
		StaminaBarWidget->SetHiddenInGame(true);
	}
}

void AGSCharacter::UpdateStaminaBarWorldLocation()
{
	if (!IsLocallyControlled())
	{
		return;
	}

	if (StaminaBarWidget == nullptr)
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC == nullptr)
	{
		return;
	}

	FVector CameraLocation;
	FRotator CameraRotation;
	PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

	const FRotator CameraYawRotation(0.f, CameraRotation.Yaw, 0.f);
	const FVector CameraRightVector = FRotationMatrix(CameraRotation).GetUnitAxis(EAxis::Y);

	const FVector NewWidgetLocation =
		GetActorLocation()
		+ CameraRightVector * StaminaBarSideOffset
		+ FVector(0.f, 0.f, StaminaBarHeightOffset);

	StaminaBarWidget->SetWorldLocation(NewWidgetLocation);
}

void AGSCharacter::OnStaminaChanged(const FOnAttributeChangeData& Data)
{
	UpdateStaminaBar(Data.NewValue, CachedMaxStamina);
	RefreshStaminaBarVisibility(Data.NewValue, CachedMaxStamina);
}

void AGSCharacter::OnMaxStaminaChanged(const FOnAttributeChangeData& Data)
{
	CachedMaxStamina = Data.NewValue;

	AGS_PlayerState* PS = GetPlayerState<AGS_PlayerState>();
	if (PS == nullptr)
	{
		return;
	}

	UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
	if (ASC == nullptr)
	{
		return;
	}

	const float CurrentStamina = ASC->GetNumericAttribute(UGS_PlayerAttributeSet::GetStaminaAttribute());

	UpdateStaminaBar(CurrentStamina, CachedMaxStamina);
	RefreshStaminaBarVisibility(CurrentStamina, CachedMaxStamina);
}

