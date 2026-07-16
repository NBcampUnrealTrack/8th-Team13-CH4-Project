#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "GS_PlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerNameChanged, const FString&, NewName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerScoreChanged, int32, NewScore);

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
	
	// Delegate Score
	FOnPlayerScoreChanged OnPlayerScoreChanged;
protected:
	
	virtual void BeginPlay() override;

	//메인에서 결과레벨로 넘어갈때 가져갈 프로퍼티
	virtual void CopyProperties(APlayerState* PlayerState) override;

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
	
#pragma region EOSLogic
public:
	UPROPERTY(ReplicatedUsing = OnRep_IsHost, BlueprintReadOnly,Category="Lobby")
	bool bIsHost = false;
	
	UFUNCTION()
	void OnRep_IsHost();
#pragma endregion 
	
#pragma region LobbySpawn
	
private:
	int32 LobbySlotIndex = -1;
	
public:
	FORCEINLINE void SetLobbySlotIndex(int32 NewIndex) {LobbySlotIndex = NewIndex;}
	FORCEINLINE int32 GetLobbySlotIndex() const {return LobbySlotIndex;}
	
#pragma endregion 
};
