#include "GS_PlayerState.h"

#include "AbilitySystemComponent.h"
#include "Gang_Squirrel/GAS/AttributeSet/GS_PlayerAttributeSet.h"
#include "Gang_Squirrel/GAS/Tags/GS_GamePlayTag.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffect.h"
#include "TimerManager.h"


AGS_PlayerState::AGS_PlayerState()
{
	//PlayerState Network Settings
	bReplicates = true;
	// Create ASC
	AbilitySystemComp = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComp->SetIsReplicated(true);
	AbilitySystemComp->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	
	// Create AttributeSet
	AttributeSet = CreateDefaultSubobject<UGS_PlayerAttributeSet>(TEXT("AttributeSet"));
}

UAbilitySystemComponent* AGS_PlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComp;
}

void AGS_PlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGS_PlayerState, PlayerNickname);
	DOREPLIFETIME(AGS_PlayerState, PlayerScore);
	DOREPLIFETIME(AGS_PlayerState, KillCount);
	DOREPLIFETIME(AGS_PlayerState,bIsHost);
}

void AGS_PlayerState::BeginPlay()
{
	Super::BeginPlay();

	/*UE_LOG(LogTemp, Warning, TEXT("[PlayerState] BeginPlay. HasAuthority: %s"),
		HasAuthority() ? TEXT("true") : TEXT("false"));*/

	StartStaminaRegen();
}

void AGS_PlayerState::SetPlayerNickname(const FString& NewName)
{
	PlayerNickname = NewName;
	OnPlayerNameChanged.Broadcast(PlayerNickname);
}

void AGS_PlayerState::OnRep_PlayerNickname()
{
	OnPlayerNameChanged.Broadcast(PlayerNickname);
}

void AGS_PlayerState::AddScore(int32 Value)
{
	if (HasAuthority() == false)
	{
		return;
	}

	PlayerScore += Value;
	
	OnRep_PlayerScore();
}

void AGS_PlayerState::OnRep_KillCount()
{
	// UE_LOG(LogTemp, Warning, TEXT("KillCount: %d"), KillCount);
}

void AGS_PlayerState::AddKillCount()
{
	KillCount++;
	OnRep_KillCount();
}

void AGS_PlayerState::OnRep_PlayerScore() 
{
	// UE_LOG(LogTemp, Warning, TEXT("PlayerScore: %d"), PlayerScore);
	
	OnPlayerScoreChanged.Broadcast(PlayerScore);
}

void AGS_PlayerState::StartStaminaRegen()
{

	if (!HasAuthority())
	{
		return;
	}

	if (!GetWorld())
	{
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(StaminaRegenTimerHandle);

	GetWorld()->GetTimerManager().SetTimer(
		StaminaRegenTimerHandle,
		this,
		&AGS_PlayerState::ApplyStaminaRegen,
		StaminaRegenInterval,
		true,
		StaminaRegenDelay
	);
}

void AGS_PlayerState::StopStaminaRegen()
{
	if (!GetWorld())
	{
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(StaminaRegenTimerHandle);
}

void AGS_PlayerState::ApplyStaminaRegen()
{

	if (!HasAuthority())
	{
		return;
	}

	if (!AbilitySystemComp)
	{	
		return;
	}

	if (!GE_StaminaRegen)
	{
		// UE_LOG(LogTemp, Error, TEXT("[StaminaRegen] GE_StaminaRegen is NULL"));
		return;
	}

	if (AbilitySystemComp->HasMatchingGameplayTag(StateTag::TAG_State_Dead))
	{
		return;
	}

	if (AbilitySystemComp->HasMatchingGameplayTag(StateTag::TAG_State_Sprinting))
	{
		return;
	}

	const float CurrentStamina = AbilitySystemComp->GetNumericAttribute(
		UGS_PlayerAttributeSet::GetStaminaAttribute()
	);

	const float MaxStamina = AbilitySystemComp->GetNumericAttribute(
		UGS_PlayerAttributeSet::GetMaxStaminaAttribute()
	);

	// UE_LOG(LogTemp, Warning, TEXT("[StaminaRegen] Before Check: %f / %f"), CurrentStamina, MaxStamina);

	if (CurrentStamina >= MaxStamina)
	{
		return;
	}

	FGameplayEffectContextHandle Context = AbilitySystemComp->MakeEffectContext();
	Context.AddSourceObject(this);

	FGameplayEffectSpecHandle SpecHandle = AbilitySystemComp->MakeOutgoingSpec(
		GE_StaminaRegen,
		1.f,
		Context
	);

	if (SpecHandle.IsValid())
	{
		AbilitySystemComp->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

		// UE_LOG(LogTemp, Warning, TEXT("[StaminaRegen] Stamina: %f / %f"),
		// 	AbilitySystemComp->GetNumericAttribute(UGS_PlayerAttributeSet::GetStaminaAttribute()),
		// 	MaxStamina
		// );
	}

}

void AGS_PlayerState::OnRep_IsHost()
{
	UE_LOG(LogTemp,Log,TEXT("[Lobby] bIsHost Replicated: %s"), bIsHost ? TEXT("true") : TEXT("false"));
}
