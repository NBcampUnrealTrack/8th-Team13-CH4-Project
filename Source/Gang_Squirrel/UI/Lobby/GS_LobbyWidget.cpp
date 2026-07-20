#include "GS_LobbyWidget.h"

#include "GS_LobbySlotWidget.h"
#include "Components/Button.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "Friend/GS_FriendListWidget.h"
#include "Gang_Squirrel/Controller/Lobby/GS_LobbyPlayerController.h"
#include "Gang_Squirrel/Game/GS_GameState.h"
#include "Gang_Squirrel/Player/GS_PlayerState.h"
#include "Gang_Squirrel/EOS/GS_GameInstance.h"

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

	if (Button_Settings)
	{
		Button_Settings->OnClicked.AddDynamic(this, &UGS_LobbyWidget::OnSettingsButtonClicked);
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
	if (!GS || GS->PlayerArray.Num() < MinPlayersToStart)
	{
		return false;
	}

	for (APlayerState* PS : GS->PlayerArray)
	{
		const AGS_PlayerState* CandidatePS = Cast<AGS_PlayerState>(PS);
		if (!CandidatePS || CandidatePS->bIsHost)
		{
			continue;
		}
		if (!CandidatePS->bIsReady)
		{
			return false;
		}
	}
	return true;
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
	
	Button_Start->SetVisibility(ESlateVisibility::Visible);
	
	if (IsLocalPlayerHost())
	{
		const bool bCanStart = CanStartGame();
		Button_Start->SetIsEnabled(bCanStart);
		
		if (Text_ButtonStart)
		{
			Text_ButtonStart->SetText(FText::FromString(TEXT("게임 시작")));
		}
	}
	else
	{
		const AGS_PlayerState* PS = GetOwningPlayerState<AGS_PlayerState>();
		const bool bReady = PS && PS->bIsReady;
		Button_Start->SetIsEnabled(true);
		if (Text_ButtonStart)
		{
			Text_ButtonStart->SetText(FText::FromString(bReady ? TEXT("준비 완료") : TEXT("준비")));
		}
	}
}

void UGS_LobbyWidget::OnStartButtonClicked()
{
	AGS_LobbyPlayerController* PC = Cast<AGS_LobbyPlayerController>(GetOwningPlayer());
	if (!PC)
	{
		return;
	}
	
	if (IsLocalPlayerHost())
	{
		PC->RequestStartGame();
	}
	else
	{
		PC->RequestToggleReady();
	}
}

void UGS_LobbyWidget::OnInviteButtonClicked()
{
	ToggleFriendList();
}

void UGS_LobbyWidget::OnSettingsButtonClicked()
{
	// ESC 토글(GS_GameInstance::ToggleSettingsWidget)과 동일한 인스턴스를 공유해서
	// 버튼으로 열든 ESC로 열든 같은 상태를 바라보게 한다.
	if (UGS_GameInstance* GSInst = GetGameInstance<UGS_GameInstance>())
	{
		GSInst->ToggleSettingsWidget(GetOwningPlayer());
	}
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
