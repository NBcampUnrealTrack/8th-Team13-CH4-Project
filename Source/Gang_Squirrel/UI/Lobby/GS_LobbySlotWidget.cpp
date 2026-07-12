#include "GS_LobbySlotWidget.h"

#include "Components/TextBlock.h"
#include "Gang_Squirrel/Player/GS_PlayerState.h"

void UGS_LobbySlotWidget::InitSlot(AGS_PlayerState* InPlayerState)
{
	if (BoundPlayerState == InPlayerState)
	{
		return;
	}
	
	if (BoundPlayerState)
	{
		BoundPlayerState->OnPlayerNameChanged.RemoveDynamic(this, &UGS_LobbySlotWidget::OnNickNameChanged);
	}
	
	BoundPlayerState = InPlayerState;
	if (!BoundPlayerState)
	{
		return;
	}
	
	BoundPlayerState->OnPlayerNameChanged.AddDynamic(this,&UGS_LobbySlotWidget::OnNickNameChanged);
	
	RefreshNickName();
	RefreshHostBadge();
}


void UGS_LobbySlotWidget::OnNickNameChanged(const FString& NewName)
{
	RefreshNickName();
}

void UGS_LobbySlotWidget::RefreshNickName()
{
	if (Text_NickName && BoundPlayerState)
	{
		Text_NickName->SetText(FText::FromString(BoundPlayerState->PlayerNickname));
	}
}

void UGS_LobbySlotWidget::RefreshHostBadge()
{
	if (HostBadge && BoundPlayerState)
	{
		HostBadge->SetVisibility(BoundPlayerState->bIsHost ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}
