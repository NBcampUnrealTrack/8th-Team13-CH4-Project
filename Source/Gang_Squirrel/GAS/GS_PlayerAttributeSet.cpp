#include "GS_PlayerAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"


UGS_PlayerAttributeSet::UGS_PlayerAttributeSet()
{
	//TODO::Refac to DataTable,Asset
	InitHealth(3.f);
	InitMaxHealth(3.f);
	InitMoveSpeed(600.f);
	InitSlowSpeedMultiplier(1.f);
}

void UGS_PlayerAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, Health)
	DOREPLIFETIME(ThisClass, MaxHealth)
	DOREPLIFETIME(ThisClass, MoveSpeed)
	DOREPLIFETIME(ThisClass, SlowSpeedMultiplier)
}

void UGS_PlayerAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	
	const FGameplayAttribute& ChangeAttribute = Data.EvaluatedData.Attribute;

#pragma region Clamping
	// HP
	if (ChangeAttribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(),0.f, GetMaxHealth()));
	}
	// SlowSpeedValue
	else if (ChangeAttribute == GetSlowSpeedMultiplierAttribute())
	{
		// TODO:: Test Value Change to DataStruct
		SetSlowSpeedMultiplier(FMath::Clamp(GetSlowSpeedMultiplier(),0.1f,1.f));
	}
#pragma endregion 
}

void UGS_PlayerAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass,Health,OldHealth);
}

void UGS_PlayerAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass,MaxHealth,OldMaxHealth);
}

void UGS_PlayerAttributeSet::OnRep_MoveSpeed(const FGameplayAttributeData& OldMoveSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass,MoveSpeed,OldMoveSpeed);
}

void UGS_PlayerAttributeSet::OnRep_SlowSpeedMultiplier(const FGameplayAttributeData& OldSlowSpeedMultiplier)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass,SlowSpeedMultiplier,OldSlowSpeedMultiplier);
}
