#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "GS_PlayerAttributeSet.generated.h"

#pragma region AttributeMacro
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)
#pragma endregion 

UCLASS()
class GANG_SQUIRREL_API UGS_PlayerAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	UGS_PlayerAttributeSet();
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;
	
#pragma region AttributeData
	// Health Data
	UPROPERTY(BlueprintReadOnly,Category="Attributes|Health",ReplicatedUsing=OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(ThisClass, Health)
	UPROPERTY(BlueprintReadOnly,Category="Attributes|Health",ReplicatedUsing=OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(ThisClass, MaxHealth)
	// Speed Data
	UPROPERTY(BlueprintReadOnly,Category="Attributes|Speed",ReplicatedUsing=OnRep_MoveSpeed)
	FGameplayAttributeData MoveSpeed;
	ATTRIBUTE_ACCESSORS(ThisClass,MoveSpeed)
	UPROPERTY(BlueprintReadOnly,Category="Attributes|Speed",ReplicatedUsing=OnRep_SlowSpeedMultiplier)
	FGameplayAttributeData SlowSpeedMultiplier;
	ATTRIBUTE_ACCESSORS(ThisClass,SlowSpeedMultiplier)
	// Combat Data
	UPROPERTY(BlueprintReadOnly,Category="Attributes|Combat")
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS(ThisClass,Damage);
#pragma endregion 
	
protected:
#pragma region ReplicatedNotifyFunc
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth);
	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);
	UFUNCTION()
	void OnRep_MoveSpeed(const FGameplayAttributeData& OldMoveSpeed);
	UFUNCTION()
	void OnRep_SlowSpeedMultiplier(const FGameplayAttributeData& OldSlowSpeedMultiplier);
#pragma endregion
	
	
	
};
