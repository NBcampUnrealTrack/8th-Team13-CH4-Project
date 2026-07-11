#include "GS_FriendEntryWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Gang_Squirrel/EOS/GS_GameInstance.h"

void UGS_FriendEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (Button_Invite)
	{
		Button_Invite->OnClicked.AddDynamic(this, &UGS_FriendEntryWidget::OnInviteClicked);
	}
}

void UGS_FriendEntryWidget::InitEntry(const FGSFriendInfo& Info)
{
	FriendIndex = Info.FriendIndex;
	
	if (Text_NickName)
	{
		Text_NickName->SetText(FText::FromString(Info.DisplayName));
	}
}

void UGS_FriendEntryWidget::OnInviteClicked()
{
	if (UGS_GameInstance* GSInst = GetGameInstance<UGS_GameInstance>())
	{
		GSInst->InviteFriend(FriendIndex);
	}
}

