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
#include "Gang_Squirrel/GAS/GA/Attack/GA_Attack.h"
#include "Components/WidgetComponent.h"
#include "Gang_Squirrel/UI/GSPlayerNameTag.h"

AGSCharacter::AGSCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

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
}

void AGSCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsRolling == false)
	{
		return;
	}

	RollingElapsedTime += DeltaTime;

	const float RollingSpeed = RollingDistance / RollingDuration;
	const FVector DeltaLocation = RollingDirection * RollingSpeed * DeltaTime;

	AddActorWorldOffset(DeltaLocation, true);

	if (RollingElapsedTime >= RollingDuration)
	{
		FinishRolling();
	}
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

	}
	
	AGS_PlayerState* PS = GetPlayerState<AGS_PlayerState>();
	if (IsValid(PS))
	{
		PS->OnPlayerNameChanged.AddDynamic(this, &ThisClass::UpdateNameTag);

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

	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
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
		EIC->BindAction(Attack, ETriggerEvent::Started, this, &ThisClass::IAAttack);
		EIC->BindAction(Sprint, ETriggerEvent::Started, this, &ThisClass::IAStartSprint);
		EIC->BindAction(Sprint, ETriggerEvent::Completed, this, &ThisClass::IAEndSprint);
		EIC->BindAction(Rolling, ETriggerEvent::Started, this, &ThisClass::IARolling);
	}
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
}

void AGSCharacter::IAAttack(const FInputActionValue& InValue)
{
	UE_LOG(LogTemp, Log, TEXT("Attack!"));
	
	AGS_PlayerState* PS = GetPlayerState<AGS_PlayerState>();
	if (PS)
	{
		PS->GetAbilitySystemComponent()->TryActivateAbilityByClass(GA_Attack);
	}
}

void AGSCharacter::IAStartSprint(const FInputActionValue& InValue)
{
	UE_LOG(LogTemp, Log, TEXT("Start Sprint!"));

	SetSprinting(true);

	if (HasAuthority() == false)
	{
		ServerSetSprinting(true);
	}
}

void AGSCharacter::IAEndSprint(const FInputActionValue& InValue)
{
	UE_LOG(LogTemp, Log, TEXT("Complite Sprint!"));

	SetSprinting(false);

	if (HasAuthority() == false)
	{
		ServerSetSprinting(false);
	}
}

void AGSCharacter::IARolling(const FInputActionValue& InValue)
{
	UE_LOG(LogTemp, Log, TEXT("Rolling!"));

	if (bIsRolling)
	{
		return;
	}
	StartRolling();
}

void AGSCharacter::SetSprinting(bool bNewSprinting)
{
	bIsSprinting = bNewSprinting;

	GetCharacterMovement()->MaxWalkSpeed = bIsSprinting ? SprintSpeed : WalkSpeed;
}

void AGSCharacter::ServerSetSprinting_Implementation(bool bNewSprinting)
{
	SetSprinting(bNewSprinting);
}

void AGSCharacter::UpdateNameTag(const FString& Newname)
{
	UGSPlayerNameTag* NameTag = Cast<UGSPlayerNameTag>(PlayerNameTagWidget->GetWidget());

	if (NameTag)
	{
		NameTag->SetNickname(Newname);
	}
}

void AGSCharacter::StartRolling()
{
	bIsRolling = true;
	RollingElapsedTime = 0.f;

	RollingDirection = GetLastMovementInputVector();

	if (RollingDirection.IsNearlyZero())
	{
		RollingDirection = GetActorForwardVector();
	}

	RollingDirection.Z = 0.f;
	RollingDirection.Normalize();

	if (AM_Roll)
	{
		PlayAnimMontage(AM_Roll);
	}

}

void AGSCharacter::FinishRolling()
{
	bIsRolling = false;
	RollingElapsedTime = 0.f;
	RollingDirection = FVector::ZeroVector;

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
		//Give Ability
		if (!PS->GetAbilitySystemComponent()->FindAbilitySpecFromClass(UGA_Attack::StaticClass()))
		{
			PS->GetAbilitySystemComponent()->GiveAbility(FGameplayAbilitySpec(GA_Attack,1));
		}
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

		if (PS->OnPlayerNameChanged.IsAlreadyBound(this, &ThisClass::UpdateNameTag) == false)
		{
			PS->OnPlayerNameChanged.AddDynamic(this, &ThisClass::UpdateNameTag);
		}

		if (PS->PlayerNickname.IsEmpty() == false)
		{
			UpdateNameTag(PS->PlayerNickname);
		}
	}
}

