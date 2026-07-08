#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_HUDWidget.generated.h"

class AGS_PlayerState;
struct FOnAttributeChangeData;
class UGS_HPCountWidget;

UCLASS()
class GANG_SQUIRREL_API UGS_HUDWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UGS_HPCountWidget> HPWidget_Mine;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UGS_HPCountWidget> HPWidget_Other;
	
private:
	// Binding PlayerState
	void BindToMyPlayerState();
	void TryFindAndBindOtherPlayer();
	void PollForPlayer();
	// Binding Attribute Change Delegate
	void OnMyHealthChanged(const FOnAttributeChangeData& Data);
	void OnOtherHealthChanged(const FOnAttributeChangeData& Data);
	
	void RefreshMyHealth();
	void RefreshOtherHealth();
	
	UPROPERTY()
	TObjectPtr<AGS_PlayerState> OtherPlayerState;
	
	FTimerHandle OtherPlayerFindTimerHandle;
	bool bMyStateBound = false;
};
