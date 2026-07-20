#include "GS_PartyWidget.h"

#include "GS_PartySlotWidget.h"
#include "Components/PanelWidget.h"
#include "Components/VerticalBoxSlot.h"
#include "Gang_Squirrel/Game/GS_GameState.h"
#include "Gang_Squirrel/Player/GS_PlayerState.h"

void UGS_PartyWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	CreateSlotPool();
	
	GetWorld()->GetTimerManager().SetTimer(RefreshTimerHandle,this,&UGS_PartyWidget::RefreshParty,0.5f,true,0.f);
}

void UGS_PartyWidget::NativeDestruct()
{
	GetWorld()->GetTimerManager().ClearTimer(RefreshTimerHandle);
	Super::NativeDestruct();
}

void UGS_PartyWidget::CreateSlotPool()
{
	if (!SlotContainer || !SlotWidgetClass)
	{
		return;
	}
	
	for (int32 i = 0; i < MaxPartySize; ++i)
	{
		UGS_PartySlotWidget* NewSlot = CreateWidget<UGS_PartySlotWidget>(this,SlotWidgetClass);
		if (NewSlot)
		{
			SlotContainer->AddChild(NewSlot);
			NewSlot->SetVisibility(ESlateVisibility::Collapsed);
			NewSlot->SetPadding(FMargin(FVector2D(0.f,10.f)));
			Slots.Add(NewSlot);
		}
	}
}

void UGS_PartyWidget::RefreshParty()
{
	AGS_GameState* GS = GetWorld()->GetGameState<AGS_GameState>();
	if (!GS)
	{
		return;
	}
	
	int32 SlotIndex = 0;
	for (APlayerState* PS : GS->PlayerArray)
	{
		AGS_PlayerState* CandidatePS = Cast<AGS_PlayerState>(PS);
		if (!CandidatePS || SlotIndex >= Slots.Num())
		{
			continue;
		}
		
		Slots[SlotIndex]->InitSlot(CandidatePS,SlotIndex);
		Slots[SlotIndex]->SetVisibility(ESlateVisibility::Visible);
		++SlotIndex;
	}
	
	for (int32 i = SlotIndex; i < Slots.Num(); ++i)
	{
		Slots[i]->SetVisibility(ESlateVisibility::Collapsed);
	}
}
