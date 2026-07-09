#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_PartySlotWidget.generated.h"


class UBorder;
struct FOnAttributeChangeData;
class UGS_HPCountWidget;
class UTextBlock;
class AGS_PlayerState;

UCLASS()
class GANG_SQUIRREL_API UGS_PartySlotWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable,Category="UI")
	void InitSlot(AGS_PlayerState* InPlayerState, int32 InSlotIndex);
	
protected:
	virtual void NativeDestruct() override;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Text_NickName;
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_Portrait;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Text_Score;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UBorder> Border_Portrait;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UGS_HPCountWidget> HPWidget;
	
	UPROPERTY(EditDefaultsOnly,Category="UI")
	TArray<FLinearColor> SlotsColors = {FLinearColor::Red,FLinearColor::Green,FLinearColor::Blue,FLinearColor::Yellow};
	
private:
	UPROPERTY()
	TObjectPtr<AGS_PlayerState> BoundPlayerState;
	
	UFUNCTION()
	void OnNicknameChanged(const FString& NewName);
	UFUNCTION()
	void OnScoreChanged(int32 NewScore);
	
	void OnHealthChanged(const FOnAttributeChangeData& Data);
	
	
	void RefreshHealth();
	void RefreshNickName();
};
