#include "GS_LobbyWidget.h"

#include "GS_LobbySlotWidget.h"
#include "Components/PanelWidget.h"
#include "Gang_Squirrel/Game/GS_GameState.h"
#include "Gang_Squirrel/Player/GS_PlayerState.h"

void UGS_LobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	CreateSlotPool();
	
	GetWorld()->GetTimerManager().SetTimer(RefreshTimerHanlde,this, &UGS_LobbyWidget::RefreshLobby, 0.5f, true, 0.f);
}

void UGS_LobbyWidget::NativeDestruct()
{
	GetWorld()->GetTimerManager().ClearTimer(RefreshTimerHanlde);
	Super::NativeDestruct();
}

bool UGS_LobbyWidget::IsLocalPlayerHost() const
{
	const AGS_PlayerState* PS = GetOwningPlayerState<AGS_PlayerState>();
	return PS && PS->bIsHost;
}

bool UGS_LobbyWidget::CanStartGame() const
{
	const AGS_GameState* GS = GetWorld()->GetGameState<AGS_GameState>();
	return GS && GS->PlayerArray.Num() >= MinPlayersToStart;
}

void UGS_LobbyWidget::CreateSlotPool()
{
	if (!SlotContainer || !SlotWidgetClass)
	{
		return;
	}
	
	for (int32 i = 0; i < MaxLobbySize; ++i)
	{
		UGS_LobbySlotWidget* NewSlot = CreateWidget<UGS_LobbySlotWidget>(this, SlotWidgetClass);
		if (NewSlot)
		{
			SlotContainer->AddChild(NewSlot);
			NewSlot->SetVisibility(ESlateVisibility::Collapsed);
			Slots.Add(NewSlot);
		}
	}
	
}

void UGS_LobbyWidget::RefreshLobby()
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
			continue;;
		}
		
		Slots[SlotIndex]->InitSlot(CandidatePS);
		Slots[SlotIndex]->SetVisibility(ESlateVisibility::Visible);
		++SlotIndex;
	}
	
	for (int32 i = SlotIndex; i < Slots.Num(); ++i)
	{
		Slots[i]->SetVisibility(ESlateVisibility::Collapsed);
	}
	
}
