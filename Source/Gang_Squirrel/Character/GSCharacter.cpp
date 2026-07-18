// Fill out your copyright notice in the Description page of Project Settings.


#include "GSCharacter.h"

#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Gang_Squirrel/Player/GS_PlayerState.h"
#include "Components/SphereComponent.h"
#include "Gang_Squirrel/GAS/AttributeSet/GS_PlayerAttributeSet.h"
#include "Gang_Squirrel/GAS/GA/Attack/GA_Attack.h"
#include "Gang_Squirrel/GAS/GA/Attack/DropKick/GA_DropKick.h"
#include "Gang_Squirrel/GAS/GA/Roll/GA_Roll.h"
#include "Gang_Squirrel/GAS/GA/Sprint/GA_Sprint.h"
#include "Gang_Squirrel/GAS/GA/Grab/GA_Grab.h"
#include "Gang_Squirrel/GAS/GA/Death/GA_PlayerDeath.h"
#include "Gang_Squirrel/GAS/Tags/GS_GamePlayTag.h"
#include "Components/WidgetComponent.h"
#include "Gang_Squirrel/Food/GSFoodBase.h"
#include "Gang_Squirrel/UI/GS_StaminaBarWidget.h"
#include "Gang_Squirrel/UI/GSPlayerNameTag.h"
#include "Gang_Squirrel/Gang_Squirrel.h"
#include "Gang_Squirrel/Food/GSCheekWidget.h"
#include "Net/UnrealNetwork.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Gang_Squirrel/EOS/GS_GameInstance.h"
#include "Gang_Squirrel/Food/Score/GSSlideWidget.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Materials/MaterialParameterCollection.h"
#include "Components/AudioComponent.h"
#include "Gang_Squirrel/DataAsset/GSFoodPrimaryDataAsset.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

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
	//Camera Lag Settings
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 10.f;
	SpringArm->bEnableCameraRotationLag = true;
	SpringArm->CameraRotationLagSpeed = 10.f;

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
	
	//Audio Component
	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
	AudioComponent->SetupAttachment(GetRootComponent());
	AudioComponent->SetVolumeMultiplier(0.4);
	// for AnimNotifyTick Func
	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
}

void AGSCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsRolling)
	{
		AddMovementInput(RollingDirection, 1.0f);
	}

	if (HasAuthority() && bIsGrabbing)
	{
		UpdateGrabTargetPosition(DeltaTime);
	}

	if (GetCharacterMovement()->IsFalling())
	{
		const float CurrentFallSpeed = FMath::Abs(GetCharacterMovement()->Velocity.Z);
		MaxFallSpeedDuringFall = FMath::Max(MaxFallSpeedDuringFall, CurrentFallSpeed);
	}

	UpdateStaminaBarWorldLocation();
}

void AGSCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	DefaultMeshRelativeLocation = GetMesh()->GetRelativeLocation();
	DefaultMeshRelativeRotation = GetMesh()->GetRelativeRotation();
	DefaultSpringArmRelativeLocation = SpringArm->GetRelativeLocation();
	DefaultSpringArmRelativeRotation = SpringArm->GetRelativeRotation();
	
	AudioComponent->Stop();
	
	SetupUpperBodyRagdoll();

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
		
		if (!PS->OnPlayerReadyChanged.IsAlreadyBound(this, &ThisClass::UpdateReadyCheck))
		{
			PS->OnPlayerReadyChanged.AddDynamic(this, &ThisClass::UpdateReadyCheck);
		}
		UpdateReadyCheck(PS->bIsReady);
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
		EIC->BindAction(Move, ETriggerEvent::Completed, this, &ThisClass::IAStopMove);
		EIC->BindAction(Move, ETriggerEvent::Canceled, this, &ThisClass::IAStopMove);
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
		EIC->BindAction(Grab, ETriggerEvent::Started, this, &ThisClass::IAStartGrab);
		EIC->BindAction(Grab, ETriggerEvent::Completed, this, &ThisClass::IAEndGrab);
		EIC->BindAction(Grab, ETriggerEvent::Canceled, this, &ThisClass::IAEndGrab
		);
	}
}

void AGSCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGSCharacter, CurrentCheekSize);
	DOREPLIFETIME(AGSCharacter, MaxCheekSize);
	DOREPLIFETIME(AGSCharacter, bIsGrabbing);
	DOREPLIFETIME(AGSCharacter, bIsGrabbed);
	DOREPLIFETIME(AGSCharacter, GrabbedTarget);
	DOREPLIFETIME(AGSCharacter, GrabOwner);
	DOREPLIFETIME(AGSCharacter, TempScore);

	
}

void AGSCharacter::IAMove(const FInputActionValue& InValue)
{
	//Grabbed
	if (bIsGrabbed)
	{
		return;
	}

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

	FVector MoveInputDirection =
		ForwardDirection * InMovementVector.X +
		RightDirection * InMovementVector.Y;

	MoveInputDirection.Z = 0.f;

	if (!MoveInputDirection.IsNearlyZero())
	{
		MoveInputDirection.Normalize();
	}

}

void AGSCharacter::IALook(const FInputActionValue& InValue)
{
	if (IsValid(Controller) == false)
	{
		return;
	}

	const FVector2D InLookVector = InValue.Get<FVector2D>();

	float SensitivityMultiplier = 1.f;
	if (UGS_GameInstance* GSInst = GetGameInstance<UGS_GameInstance>())
	{
		SensitivityMultiplier = GSInst->MouseSensitivity / 100.f;   // 50이 기본값(배율 1.0)
	}

	AddControllerYawInput(InLookVector.X * SensitivityMultiplier);
	AddControllerPitchInput(InLookVector.Y * SensitivityMultiplier);
}

void AGSCharacter::IAInteract(const FInputActionValue& InValue)
{
	// UE_LOG(LogTemp, Log, TEXT("Interact!"));
	
	if (CurrentCheekSize >= MaxCheekSize) return;
	
	if (bIsEating) return;

	bIsEating = true;

	// EatingAnimation
	if (AM_Eat)
	{
		PlayAnimMontage(AM_Eat);
		AudioComponent->Play();
	}

	Server_SetEating(true);
}

void AGSCharacter::IAStopInteract(const FInputActionValue& InValue)
{
	// UE_LOG(LogTemp, Log, TEXT("StopInteract!"));


	if (!bIsEating) return;

	bIsEating = false;

	if (AM_Eat)
	{
		StopAnimMontage(AM_Eat);
		AudioComponent->Stop();
	}

	Server_SetEating(false);
}

void AGSCharacter::Server_SetEating_Implementation(bool bEating)
{
	// UE_LOG(LogTemp, Log, TEXT("SetEating!"));

	bIsEating = bEating;

	Multicast_SetEatingAnimation(bEating);
}

void AGSCharacter::InflateCheeks(float Value)
{
	if (HasAuthority() || IsLocallyControlled())
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

	if (IsLocallyControlled()
		&& CurrentCheekSize >= MaxCheekSize
		&& !bCheekFullTutorialShown)
	{
		bCheekFullTutorialShown = true;
		BP_OnCheekFull();
	}
	
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
	
	//UE_LOG(LogTemp, Error, TEXT("ResetCheekSize!"));
}

void AGSCharacter::AddMaxCheekSize(float Value)
{
	MaxCheekSize += Value;
	
	Multicast_InflateCheeks(0.f);
}

void AGSCharacter::IAAttack(const FInputActionValue& InValue)
{
	// UE_LOG(LogTemp, Log, TEXT("Attack!"));
	
	AGS_PlayerState* PS = GetPlayerState<AGS_PlayerState>();
	if (!PS)
	{
		return;
	}
	UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	if (GetCharacterMovement()->IsFalling())
	{
		if (bHasDropKickedThisJump)
		{
			return;
		}

		bHasDropKickedThisJump = true;
		ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(AbilityTag::TAG_Ability_DropKick));
		return;
	}

	for (FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
	{
		if (Spec.IsActive() && Spec.Ability && Spec.Ability->AbilityTags.HasTag(AbilityTag::TAG_Ability_Attack))
		{
			if (!HasAuthority())
			{
				if (UGA_Attack* LocalPredicted = Cast<UGA_Attack>(Spec.GetPrimaryInstance()))
				{
					LocalPredicted->RequestComboInput();
				}
			}
			ServerRequestComboAttack();
			return;
		}
	}

	ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(AbilityTag::TAG_Ability_Attack));
}

void AGSCharacter::IAStartSprint(const FInputActionValue& InValue)
{
	// UE_LOG(LogTemp, Log, TEXT("Start Sprint!"));

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
	// UE_LOG(LogTemp, Log, TEXT("Complete Sprint!"));

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
	// UE_LOG(LogTemp, Log, TEXT("Rolling!"));

	AGS_PlayerState* PS = GetPlayerState<AGS_PlayerState>();
	if (PS)
	{
		PS->GetAbilitySystemComponent()->TryActivateAbilitiesByTag(FGameplayTagContainer(AbilityTag::TAG_Ability_Roll));
	}
}

void AGSCharacter::IAStartGrab(const FInputActionValue& InValue)
{
	if (!IsLocallyControlled())
	{
		return;
	}

	if (bIsGrabbed)
	{
		return;
	}

	AGS_PlayerState* PS = GetPlayerState<AGS_PlayerState>();
	if (PS == nullptr)
	{
		return;
	}

	UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
	if (ASC == nullptr || GA_Grab == nullptr)
	{
		return;
	}

	ASC->TryActivateAbilityByClass(GA_Grab);
}

void AGSCharacter::IAEndGrab(const FInputActionValue& InValue)
{
	if (!IsLocallyControlled())
	{
		return;
	}

	if (HasAuthority())
	{
		CancelGrabAbility();
	}
	else
	{
		ServerCancelGrabAbility();
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

void AGSCharacter::UpdateReadyCheck(bool bReady)
{
	UGSPlayerNameTag* NameTag = Cast<UGSPlayerNameTag>(PlayerNameTagWidget->GetWidget());
	if (NameTag)
	{
		NameTag->SetReadyState(bReady);
	}
}

void AGSCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	bHasDropKickedThisJump = false;

	if (HasAuthority() == false)
	{
		MaxFallSpeedDuringFall = 0.f;
		return;
	}

	const float FallSpeed = MaxFallSpeedDuringFall;
	MaxFallSpeedDuringFall = 0.f;

	if (FallSpeed < FallDamageVelocityThreshold)
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (ASC == nullptr || GE_FallDamage == nullptr)
	{
		return;
	}

	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	Context.AddSourceObject(this);

	FGameplayEffectSpecHandle SpecHandle =
		ASC->MakeOutgoingSpec(GE_FallDamage, 1.f, Context);

	if (SpecHandle.IsValid())
	{
		ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}

	UE_LOG(LogTemp, Warning, TEXT("[FallDamage] FallSpeed: %.2f"), FallSpeed);//check for velocity
}

void AGSCharacter::Client_ShowFallingHazardTutorial_Implementation()
{
	if (bHasShownFallingHazardTutorial)
	{
		return;
	}

	bHasShownFallingHazardTutorial = true;

	BP_OnFallingHazardTargeted();
}

void AGSCharacter::IAStopMove(const FInputActionValue& InValue)
{
	//direction
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
	const float SafeSlowMultiplier =
		FMath::Clamp(CachedSlowSpeedMultiplier, 0.1f, 1.f);

	const float FinalMultiplier = GetFinalMoveSpeedMultiplier();

	const float FinalSpeed =
		SafeMoveSpeed *
		SafeSlowMultiplier *
		FinalMultiplier;

	GetCharacterMovement()->MaxWalkSpeed = FinalSpeed;

}

void AGSCharacter::StartRolling(const FVector& InRollingDirection)
{// server
	if (bIsRolling)
	{
		return;
	}

	bIsRolling = true;

	RollingDirection = InRollingDirection;
	RollingDirection.Z = 0.f;
	RollingDirection.Normalize();

	UpdateMaxWalkSpeedFromAttribute();

	if (IsLocallyControlled() && AM_Roll)
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

	MulticastPlayRollMontage();

	// UE_LOG(LogTemp, Log, TEXT("Start Rolling"));
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
		// PS->GetAbilitySystemComponent()->AddLooseGameplayTag(TeamTag::TAG_Team_Player);

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
		if (!PS->GetAbilitySystemComponent()->FindAbilitySpecFromClass(UGA_Grab::StaticClass()))
		{
			PS->GetAbilitySystemComponent()->GiveAbility(FGameplayAbilitySpec(GA_Grab, 1));
		}
		if (!PS->GetAbilitySystemComponent()->FindAbilitySpecFromClass(UGA_DropKick::StaticClass()))
		{
			PS->GetAbilitySystemComponent()->GiveAbility(FGameplayAbilitySpec(GA_DropKick, 1));
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
		
		if (!PS->OnPlayerReadyChanged.IsAlreadyBound(this, &ThisClass::UpdateReadyCheck))
		{
			PS->OnPlayerReadyChanged.AddDynamic(this, &ThisClass::UpdateReadyCheck);
		}
		UpdateReadyCheck(PS->bIsReady);
		
		
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

void AGSCharacter::Server_NotifyFoodEaten_Implementation(AGSFoodBase* EatenFood, AGSCharacter*  EatingCharacter)
{
	if (!IsValid(EatenFood) || !IsValid(EatingCharacter)) return;
	
	if (IsValid(EatenFood->FoodData))
	{
		EatingCharacter->InflateCheeks(EatenFood->FoodData->SquirrelScale);
	}
	
	EatenFood->SetCurrentCharacter(EatingCharacter);
	
	EatenFood->Eaten();
}

void AGSCharacter::Server_NotifyAddScore_Implementation(int32 Score)
{
	
	// UE_LOG(LogTemp, Warning, TEXT("Oer"));
	AGS_PlayerState* PS = GetPlayerState<AGS_PlayerState>();
	if (PS)
	{
		PS->AddScore(Score);
		
		// UE_LOG(LogTemp, Warning, TEXT("UpdateScore"));
	}
}

void AGSCharacter::ShowSlideWidget(AGSCharacter* CurrentCharacter, int32 Score) const
{
	UGSSlideWidget* CurrentSlideWidget = CreateWidget<UGSSlideWidget>(GetWorld(),SlideWidgetClass);
		
	CurrentSlideWidget->AddToViewport();
	CurrentSlideWidget->UpdateSlideWidget(Score);
}

void AGSCharacter::Server_AddTempScore_Implementation(int32 Amount)
{
	AddTempScore(Amount);
	
	UE_LOG(LogTemp, Log, TEXT("[Server] 캐릭터 %s의 임시 점수가 %d 더해졌습니다. (현재 TempScore: %d)"), *GetName(), Amount, TempScore);
}

void AGSCharacter::AddTempScore(int32 Value)
{
	TempScore += Value;
}

void AGSCharacter::ResetTempScore()
{
	TempScore = 0;
}

// GA_Death Callback Func : Temp Logic
void AGSCharacter::OnDeathStateTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	GetCapsuleComponent()->SetCollisionEnabled(NewCount <= 0 ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
	
	TempScore = 0;
	CurrentCheekSize = 0;
	MaxFallSpeedDuringFall = 0.f; // 낙하 가속도 0으로 초기화
	Multicast_InflateCheeks(0.f);

	if (IsLocallyControlled())
	{
		float TargetGrayValue = (NewCount > 0) ? 1.0f : 0.0f; //포스트 프로세스 머티리얼 파라미터 변수
		UMaterialParameterCollection* MyMPC = Cast<UMaterialParameterCollection>(
			StaticLoadObject(UMaterialParameterCollection::StaticClass(), nullptr, TEXT(
				"/Script/Engine.MaterialParameterCollection'/Game/ExternalContent/LevelPrototyping/Materials/MPC_ScreenEffects.MPC_ScreenEffects'")));
		
		if (GetWorld() && MyMPC) //방어 코드
		{
			UKismetMaterialLibrary::SetScalarParameterValue(GetWorld(), MyMPC, FName("GrayAlpha"), TargetGrayValue);
		}
	}
}

void AGSCharacter::PlayVictoryMontage()
{
	if(AM_Victory)
	{
		PlayAnimMontage(AM_Victory);
	}
}

// GA_Attack ComboAttackWindow
void AGSCharacter::ServerRequestComboAttack_Implementation()
{
	AGS_PlayerState* PS = GetPlayerState<AGS_PlayerState>();
	if (!PS)
	{
		return;
	}
	
	UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	for (FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
	{
		if (Spec.IsActive() && Spec.Ability && Spec.Ability->AbilityTags.HasTag(AbilityTag::TAG_Ability_Attack))
		{
			if (UGA_Attack* AttackAbility = Cast<UGA_Attack>(Spec.GetPrimaryInstance()))
			{
				AttackAbility->RequestComboInput();
			}
			return;
		}
	}
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

//Grab
void AGSCharacter::ServerCancelGrabAbility_Implementation()
{
	CancelGrabAbility();
}

void AGSCharacter::Multicast_SetGrabAnimation_Implementation(bool bGrab)
{
	if (AM_Grab == nullptr)
	{
		return;
	}

	if (bGrab)
	{
		PlayAnimMontage(AM_Grab);
	}
	else
	{
		StopAnimMontage(AM_Grab);
	}
}

void AGSCharacter::OnRep_IsGrabbed()
{
	SetGrabbedMovementState(bIsGrabbed);
}

void AGSCharacter::SetGrabbedMovementState(bool bNewGrabbed)
{
	UCharacterMovementComponent* MovementComp = GetCharacterMovement();
	if (MovementComp == nullptr)
	{
		return;
	}

	if (bNewGrabbed)
	{
		if (!bCachedGrabbedMovementSettings)
		{
			CachedGrabbedGroundFriction =
				MovementComp->GroundFriction;

			CachedGrabbedBrakingDeceleration =
				MovementComp->BrakingDecelerationWalking;

			CachedGrabbedMaxAcceleration =
				MovementComp->MaxAcceleration;

			bCachedGrabbedMovementSettings = true;
		}

		/*
		 * 잡힌 동안 자기 입력으로 가속하거나,
		 * 브레이킹 때문에 전달받은 속도가 즉시 줄어드는 것을 방지한다.
		 */
		MovementComp->MaxAcceleration = 0.f;
		MovementComp->GroundFriction = 0.f;
		MovementComp->BrakingDecelerationWalking = 0.f;

		if (Controller)
		{
			Controller->SetIgnoreMoveInput(true);
		}
	}
	else
	{
		if (bCachedGrabbedMovementSettings)
		{
			MovementComp->GroundFriction =
				CachedGrabbedGroundFriction;

			MovementComp->BrakingDecelerationWalking =
				CachedGrabbedBrakingDeceleration;

			MovementComp->MaxAcceleration =
				CachedGrabbedMaxAcceleration;

			bCachedGrabbedMovementSettings = false;
		}

		if (Controller)
		{
			Controller->SetIgnoreMoveInput(false);
		}

		// 잡기가 풀린 뒤 밀기 속도가 계속 남지 않게 한다.
		MovementComp->Velocity.X = 0.f;
		MovementComp->Velocity.Y = 0.f;
	}
}

void AGSCharacter::CancelGrabAbility()
{
	AGS_PlayerState* PS = GetPlayerState<AGS_PlayerState>();
	if (PS == nullptr)
	{
		return;
	}

	UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
	if (ASC == nullptr || GA_Grab == nullptr)
	{
		return;
	}

	FGameplayAbilitySpec* GrabSpec = ASC->FindAbilitySpecFromClass(GA_Grab);
	if (GrabSpec == nullptr)
	{
		return;
	}

	if (GrabSpec->IsActive())
	{
		ASC->CancelAbilityHandle(GrabSpec->Handle);
	}
}

void AGSCharacter::StartGrabTarget(AGSCharacter* TargetCharacter)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!IsValid(TargetCharacter))
	{
		return;
	}

	if (TargetCharacter == this)
	{
		return;
	}

	if (TargetCharacter->bIsGrabbed)
	{
		return;
	}

	bIsGrabbing = true;
	GrabbedTarget = TargetCharacter;

	TargetCharacter->bIsGrabbed = true;
	TargetCharacter->GrabOwner = this;

	/*
	 * 처음에는 실제 상대가 있는 방향을 사용한다.
	 * 그래서 잡기 시작 순간 상대가 갑자기 캐릭터 정면을
	 * 가로질러 이동하려는 현상을 줄인다.
	 */
	CurrentGrabPushDirection =
		TargetCharacter->GetActorLocation() - GetActorLocation();

	CurrentGrabPushDirection.Z = 0.f;

	if (CurrentGrabPushDirection.IsNearlyZero())
	{
		CurrentGrabPushDirection = GetActorForwardVector();
	}

	CurrentGrabPushDirection.Normalize();

	/*
 * 잡는 캐릭터와 잡힌 캐릭터가
 * 서로의 이동을 막지 않게 한다.
 */
	MoveIgnoreActorAdd(TargetCharacter);
	TargetCharacter->MoveIgnoreActorAdd(this);


	// 서버에서도 즉시 이동 제한 적용
	TargetCharacter->SetGrabbedMovementState(true);

	Multicast_SetGrabAnimation(true);
}

void AGSCharacter::UpdateGrabTargetPosition(float DeltaTime)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!IsValid(GrabbedTarget))
	{
		StopGrab();
		return;
	}

	UCharacterMovementComponent* GrabberMovement =
		GetCharacterMovement();

	UCharacterMovementComponent* TargetMovement =
		GrabbedTarget->GetCharacterMovement();

	if (GrabberMovement == nullptr || TargetMovement == nullptr)
	{
		StopGrab();
		return;
	}

	/*
	 * 1. 잡는 플레이어의 현재 수평 속도
	 *
	 * 위치 차이를 DeltaTime으로 나누지 않고
	 * CharacterMovement가 관리하는 Velocity를 그대로 사용
	 */
	FVector GrabberHorizontalVelocity = GrabberMovement->Velocity;
	GrabberHorizontalVelocity.Z = 0.f;

	/*
	 * 2. 밀리는 방향
	 *
	 * 이동 중이면 실제 이동 방향을 사용한다.
	 * 멈춘 상태라면 캐릭터 전방 방향을 사용한다.
	 */
	/*FVector PushDirection = GrabberHorizontalVelocity.GetSafeNormal2D();

	if (PushDirection.IsNearlyZero())
	{
		PushDirection = GetActorForwardVector();
		PushDirection.Z = 0.f;
		PushDirection.Normalize();
	}*/

	/*
	 * 속도가 충분할 때만 새로운 이동 방향을 계산한다.
	 *
	 * 아주 느린 속도나 감속 과정에서 방향이 흔들리는 것을 막는다.
	 */
	if (GrabberHorizontalVelocity.Size2D() > GrabDirectionUpdateMinSpeed)
	{
		FVector DesiredPushDirection =
			GrabberHorizontalVelocity.GetSafeNormal2D();

		/*
		 * 현재 밀기 방향과 새 이동 방향의 각도 차이 확인
		*/
		const float DirectionDot = FVector::DotProduct(
			CurrentGrabPushDirection,
			DesiredPushDirection
		);

		/*
		 * Dot -0.5는 약 120도 차이
		 */
		if (DirectionDot < -0.5f)
		{
			StopGrab();
			return;
		}
		/*
		 * 현재 방향을 새 방향으로 바로 바꾸지 않고
		 * 부드럽게 보간한다.
		 */
		CurrentGrabPushDirection = FMath::VInterpTo(
			CurrentGrabPushDirection,
			DesiredPushDirection,
			DeltaTime,
			GrabDirectionInterpSpeed
		);

		CurrentGrabPushDirection.Z = 0.f;
		CurrentGrabPushDirection.Normalize();
	}

	FVector PushDirection = CurrentGrabPushDirection;

	if (PushDirection.IsNearlyZero())
	{
		PushDirection = GetActorForwardVector();
		PushDirection.Z = 0.f;
		PushDirection.Normalize();

		CurrentGrabPushDirection = PushDirection;
	}

	/*
	 * 3. 두 캡슐이 겹치지 않는 최소 거리 계산
	 *
	 * 고정값 18이 아니라:
	 * 잡는 캐릭터 캡슐 반지름
	 * + 잡힌 캐릭터 캡슐 반지름
	 * + 작은 여유 간격
	 */
	const float GrabberCapsuleRadius =
		GetCapsuleComponent()->GetScaledCapsuleRadius();

	const float TargetCapsuleRadius =
		GrabbedTarget->GetCapsuleComponent()
		->GetScaledCapsuleRadius();

	const float ContactDistance =
		GrabberCapsuleRadius
		+ TargetCapsuleRadius
		+ GrabContactGap;

	/*
	 * 4. 잡힌 플레이어가 있어야 할 목표 위치
	 */
	const FVector DesiredTargetLocation =
		GetActorLocation()
		+ PushDirection * ContactDistance;

	FVector PositionError =
		DesiredTargetLocation
		- GrabbedTarget->GetActorLocation();

	PositionError.Z = 0.f;

	/*
	 * 5. 잡는 사람의 속도를 기본 속도로 사용
	 *
	 * 따라서 상대가 점점 멀어지지 않고
	 * 기본적으로 나와 같은 속도로 움직임
	 */
	FVector FinalVelocity = GrabberHorizontalVelocity;

	/*
	 * 6. 거리가 벌어진 경우에만 약한 보정 추가
	 *
	 * 항상 강제로 정렬하지 않고 DeadZone을 둬서
	 * 작은 위치 오차에 의한 떨림을 막기
	 */
	const float PositionErrorSize = PositionError.Size2D();

	if (PositionErrorSize > GrabPositionDeadZone)
	{
		FVector CorrectionVelocity =
			PositionError * GrabFollowCorrectionStrength;

		CorrectionVelocity.Z = 0.f;

		if (CorrectionVelocity.Size2D()
			> MaxGrabFollowCorrectionSpeed)
		{
			CorrectionVelocity =
				CorrectionVelocity.GetSafeNormal2D()
				* MaxGrabFollowCorrectionSpeed;
		}

		FinalVelocity += CorrectionVelocity;
	}

	/*
	 * LaunchCharacter를 호출하지 않는다.
	 * 잡힌 캐릭터의 수평 속도만 갱신하고
	 * 점프나 낙하에 필요한 Z 속도는 유지한다.
	 */
	TargetMovement->Velocity.X = FinalVelocity.X;
	TargetMovement->Velocity.Y = FinalVelocity.Y;
}

void AGSCharacter::StopGrab()
{
	if (!HasAuthority())
	{
		return;
	}

	bIsGrabbing = false;

	Multicast_SetGrabAnimation(false);

	if (IsValid(GrabbedTarget))
	{
		MoveIgnoreActorRemove(GrabbedTarget);
		GrabbedTarget->MoveIgnoreActorRemove(this);

		GrabbedTarget->bIsGrabbed = false;
		GrabbedTarget->GrabOwner = nullptr;

		// 서버에서 즉시 원래 이동 설정 복구
		GrabbedTarget->SetGrabbedMovementState(false);
	}

	GrabbedTarget = nullptr;

	UpdateMaxWalkSpeedFromAttribute();
}

void AGSCharacter::UpdateSlideWidget_Implementation(int32 Value)
{
	UGSSlideWidget* CurrentSlideRewardWidget = CreateWidget<UGSSlideWidget>(GetWorld(), SlideWidgetRewardClass);
	
	if (CurrentSlideRewardWidget)
	{
		CurrentSlideRewardWidget->AddToViewport();
		
		CurrentSlideRewardWidget->UpdateText(Value);
	}
}

void AGSCharacter::SetupUpperBodyRagdoll()
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

void AGSCharacter::Applyknockdown(FVector Impulse, FName BoneName, float Duration)
{
	if (!HasAuthority())
	{
		return;
	}
	
	NetMulticast_SetFullRagdollEnable(true);
	NetMulticast_ApplyRagdollImpulse(Impulse,BoneName);
	NetMulticast_SetCameraFollowRagdoll(true);
	
	GetWorld()->GetTimerManager().SetTimer(KnockdownRecoveryTimerHandle,this,&AGSCharacter::RecoverFromKnockdown,Duration,false);
}

void AGSCharacter::RecoverFromKnockdown()
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		if (ASC->HasMatchingGameplayTag(StateTag::TAG_State_Dead))
		{
			return;
		}
	}
	
	RepositionCapsuleToRagdoll();
	NetMulticast_SetCameraFollowRagdoll(false);
	NetMulticast_SetFullRagdollEnable(false);
}

void AGSCharacter::RepositionCapsuleToRagdoll()
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
	
	SetActorLocationAndRotation(NewLocation, NewRotation,false,nullptr,ETeleportType::TeleportPhysics);
}

void AGSCharacter::NetMulticast_ApplyRagdollImpulse_Implementation(FVector Impulse, FName BoneName)
{
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->AddImpulseAtLocation(Impulse,MeshComp->GetBoneLocation(BoneName),BoneName);
	}
}

void AGSCharacter::NetMulticast_SetFullRagdollEnable_Implementation(bool bEnable)
{
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!MeshComp)
	{
		return;
	}
	
	if (bEnable)
	{
		MeshComp->RecreatePhysicsState();
		MeshComp->SetAllBodiesSimulatePhysics(true);
		for (FBodyInstance* Body : MeshComp->Bodies)
		{
			if (Body)
			{
				Body->SetUseCCD(true);
			}
		}
		MeshComp->WakeAllRigidBodies();
	}
	else
	{
		MeshComp->SetAllBodiesSimulatePhysics(false);
		MeshComp->SetRelativeLocationAndRotation(DefaultMeshRelativeLocation,DefaultMeshRelativeRotation);
		MeshComp->TickAnimation(0.f, false);
		MeshComp->RefreshBoneTransforms();
		SetupUpperBodyRagdoll();
	}
}

void AGSCharacter::NetMulticast_SetCameraFollowRagdoll_Implementation(bool bEnable)
{
	if (!IsLocallyControlled() || !SpringArm)
	{
		return;
	}
	
	if (bEnable)
	{
		SpringArm->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepWorldTransform, RagdollStartBone);
	}
	else
	{
		SpringArm->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		SpringArm->SetRelativeLocationAndRotation(DefaultSpringArmRelativeLocation,DefaultSpringArmRelativeRotation);
	}
}

void AGSCharacter::Client_PlayAttackHitSound_Implementation()
{
	if (!AttackHitSound)
	{
		return;
	}

	UGameplayStatics::PlaySound2D(this, AttackHitSound);
}