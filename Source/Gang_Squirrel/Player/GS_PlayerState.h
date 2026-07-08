#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "GS_PlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerNameChanged, const FString&, NewName);

class UGameplayEffect;
class UGS_PlayerAttributeSet;

UCLASS()
class GANG_SQUIRREL_API AGS_PlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
public:
	AGS_PlayerState();
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;
protected:
	
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComp;
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UGS_PlayerAttributeSet> AttributeSet;

public:
	//Input NickName
	UPROPERTY(ReplicatedUsing = OnRep_PlayerNickname, BlueprintReadOnly, Category = "Player")
	FString PlayerNickname;

	//Changed NickName Delegate
	FOnPlayerNameChanged OnPlayerNameChanged;

public:
	//Nickname Setting
	void SetPlayerNickname(const FString& NewName);

	UFUNCTION()
	void OnRep_PlayerNickname();
	
private:
	//Score
	UPROPERTY(ReplicatedUsing=OnRep_PlayerScore)
	int32 PlayerScore = 0;

	UPROPERTY(ReplicatedUsing = OnRep_KillCount)
	int32 KillCount = 0;
public:
	UFUNCTION()
	void OnRep_PlayerScore();
	
	void AddScore(int32 Value);
	
	FORCEINLINE int32 GetPlayerScore() const { return PlayerScore; }

	// AI KillCount
	UFUNCTION()
	void OnRep_KillCount();

	void AddKillCount();

	FORCEINLINE int32 GetKillCount() const { return KillCount; }

protected:
	//Stamina
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
	TSubclassOf<UGameplayEffect> GE_StaminaRegen;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
	float StaminaRegenInterval = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
	float StaminaRegenDelay = 1.0f;

	FTimerHandle StaminaRegenTimerHandle;

public:
	void StartStaminaRegen();
	void StopStaminaRegen();

private:
	void ApplyStaminaRegen();
};
