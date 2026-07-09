#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_HUDWidget.generated.h"

class UGS_PartyWidget;
class AGS_PlayerState;
struct FOnAttributeChangeData;
class UGS_HPCountWidget;
class UPanelWidget;

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
	
	//PartyWidget
	UPROPERTY(EditDefaultsOnly,Category="UI")
	TSubclassOf<UGS_PartyWidget> PartyWidgetClass;
	
private:
	// Binding PlayerState
	void BindToMyPlayerState();
	void PollForPlayer();
	// Binding Attribute Change Delegate
	void OnMyHealthChanged(const FOnAttributeChangeData& Data);
	
	void RefreshMyHealth();
	
	FTimerHandle MyStateBindTimerHandle;
	bool bMyStateBound = false;
};
