#include "GS_FriendListWidget.h"

#include "GS_FriendEntryWidget.h"
#include "Components/PanelWidget.h"
#include "Gang_Squirrel/EOS/GS_GameInstance.h"

void UGS_FriendListWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (UGS_GameInstance* GSInst = GetGameInstance<UGS_GameInstance>())
	{
		GSInst->OnGSFriendsListComplete.AddDynamic(this,&UGS_FriendListWidget::OnFriendsListComplete);
		GSInst->RequestFriendsList();
	}
		
}

void UGS_FriendListWidget::NativeDestruct()
{
	if (UGS_GameInstance* GSInst = GetGameInstance<UGS_GameInstance>())
	{
		GSInst->OnGSFriendsListComplete.RemoveDynamic(this, &UGS_FriendListWidget::OnFriendsListComplete);
	}
	
	Super::NativeDestruct();
}

void UGS_FriendListWidget::OnFriendsListComplete(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		RefreshEntries();
	}
}

void UGS_FriendListWidget::RefreshEntries()
{
	if (!EntryContainer || !EntryWidgetClass)
	{
		return;
	}
	
	UGS_GameInstance* GSInstance = GetGameInstance<UGS_GameInstance>();
	if (!GSInstance)
	{
		return;
	}
	
	TArray<FGSFriendInfo> Friends;
	GSInstance->GetCachedFriends(Friends);
	
	EntryContainer->ClearChildren();

	for (const FGSFriendInfo& Info : Friends)
	{
		UGS_FriendEntryWidget* Entry = CreateWidget<UGS_FriendEntryWidget>(this, EntryWidgetClass);
		if (Entry)
		{
			Entry->InitEntry(Info);
			EntryContainer->AddChild(Entry);
		}
	}
	
}
