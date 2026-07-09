#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_PartyWidget.generated.h"

class UGS_PartySlotWidget;
class UPanelWidget;


UCLASS()
class GANG_SQUIRREL_API UGS_PartyWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPanelWidget> SlotContainer;
	
	UPROPERTY(EditDefaultsOnly,Category="UI")
	TSubclassOf<UGS_PartySlotWidget> SlotWidgetClass;
	UPROPERTY(EditDefaultsOnly,Category="UI")
	int32 MaxPartySize = 4;
	
private:
	UPROPERTY()
	TArray<TObjectPtr<UGS_PartySlotWidget>> Slots;
	
	FTimerHandle RefreshTimerHandle;
	
	void CreateSlotPool();
	void RefreshParty();
};
