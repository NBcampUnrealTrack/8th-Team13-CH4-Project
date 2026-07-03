#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "GS_PlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerNameChanged, const FString&, NewName);


class UGS_PlayerAttributeSet;

UCLASS()
class GANG_SQUIRREL_API AGS_PlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
public:
	AGS_PlayerState();
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;
	
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
	
public:
	
	UFUNCTION()
	void OnRep_PlayerScore();
	
	void AddScore(int32 Value);
	
	FORCEINLINE int32 GetPlayerScore() const { return PlayerScore; }
};
