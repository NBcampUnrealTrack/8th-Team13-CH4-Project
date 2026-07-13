#include "GS_LobbyWidget.h"

#include "GS_LobbySlotWidget.h"
#include "Components/Button.h"
#include "Components/PanelWidget.h"
#include "Friend/GS_FriendListWidget.h"
#include "Gang_Squirrel/Controller/Lobby/GS_LobbyPlayerController.h"
#include "Gang_Squirrel/Game/GS_GameState.h"
#include "Gang_Squirrel/Player/GS_PlayerState.h"

void UGS_LobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CreateSlotPool();
	
	if (Button_Start)
	{
		Button_Start->OnClicked.AddDynamic(this,&UGS_LobbyWidget::OnStartButtonClicked);
	}
	
	if (Button_Invite)
	{
		Button_Invite->OnClicked.AddDynamic(this,&UGS_LobbyWidget::OnInviteButtonClicked);
	}
	
	GetWorld()->GetTimerManager().SetTimer(RefreshTimerHandle,this, &UGS_LobbyWidget::RefreshLobby, 0.5f, true, 0.f);
}

void UGS_LobbyWidget::NativeDestruct()
{
	GetWorld()->GetTimerManager().ClearTimer(RefreshTimerHandle);
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
	
	if (SlotContainer)
	{
		SlotContainer->SetVisibility(ESlateVisibility::Visible);
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
	
	RefreshStartButton();
}

void UGS_LobbyWidget::RefreshStartButton()
{
	if (!Button_Start)
	{
		return;
	}

	const bool bShowButton = IsLocalPlayerHost() && CanStartGame();
	Button_Start->SetVisibility(bShowButton? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	Button_Start->SetIsEnabled(bShowButton);
}

void UGS_LobbyWidget::OnStartButtonClicked()
{
	if (AGS_LobbyPlayerController* PC = Cast<AGS_LobbyPlayerController>(GetOwningPlayer()))
	{
		PC->RequestStartGame();
	}
}

void UGS_LobbyWidget::OnInviteButtonClicked()
{
	ToggleFriendList();
}

void UGS_LobbyWidget::ToggleFriendList()
{
	if (!FriendListContainer || !FriendWidgetClass)
	{
		return;
	}
	
	if (!FriendListWidgetInst)
	{
		FriendListWidgetInst = CreateWidget<UGS_FriendListWidget>(this,FriendWidgetClass);
		if (!FriendListWidgetInst)
		{
			return;
		}
		FriendListContainer->AddChild(FriendListWidgetInst);
		FriendListWidgetInst->SetVisibility(ESlateVisibility::Visible);
		return;
	}
	
	const bool bIsVisible = FriendListWidgetInst->GetVisibility() == ESlateVisibility::Visible;
	FriendListWidgetInst->SetVisibility(bIsVisible ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
}
