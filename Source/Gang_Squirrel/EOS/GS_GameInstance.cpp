#include "GS_GameInstance.h"

#include "Online.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlinePresenceInterface.h"
#include "Kismet/GameplayStatics.h"

void UGS_GameInstance::Init()
{
	Super::Init();

	if (IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld()))
	{
		SessionInterface = OnlineSub->GetSessionInterface();
		IdentityInterface = OnlineSub->GetIdentityInterface();
		FriendsInterface = OnlineSub->GetFriendsInterface();
	}

	if (SessionInterface.IsValid())
	{
		SessionInviteReceivedDelegateHandle = SessionInterface->AddOnSessionInviteReceivedDelegate_Handle(
			FOnSessionInviteReceivedDelegate::CreateUObject(this, &UGS_GameInstance::HandleSessionInviteReceived));

		SessionUserInviteAcceptedDelegateHandle = SessionInterface->AddOnSessionUserInviteAcceptedDelegate_Handle(
			FOnSessionUserInviteAcceptedDelegate::CreateUObject(this, &UGS_GameInstance::HandleSessionUserInviteAccepted));
	}
}

void UGS_GameInstance::Shutdown()
{
	if (IdentityInterface.IsValid())
	{
		IdentityInterface->ClearOnLoginCompleteDelegate_Handle(0, LoginCompleteDelegateHandle);
	}

	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
		SessionInterface->ClearOnSessionInviteReceivedDelegate_Handle(SessionInviteReceivedDelegateHandle);
		SessionInterface->ClearOnSessionUserInviteAcceptedDelegate_Handle(SessionUserInviteAcceptedDelegateHandle);
	}

	Super::Shutdown();
}

void UGS_GameInstance::Login()
{
	if (!IdentityInterface.IsValid())
	{
		OnGSLoginComplete.Broadcast(false);
		return;
	}

	if (IdentityInterface->GetLoginStatus(0) == ELoginStatus::LoggedIn)
	{
		OnGSLoginComplete.Broadcast(true);
		return;
	}

	LoginCompleteDelegateHandle = IdentityInterface->AddOnLoginCompleteDelegate_Handle(
		0, FOnLoginCompleteDelegate::CreateUObject(this, &UGS_GameInstance::HandleLoginComplete));

	if (!IdentityInterface->AutoLogin(0))
	{
		IdentityInterface->ClearOnLoginCompleteDelegate_Handle(0, LoginCompleteDelegateHandle);
		OnGSLoginComplete.Broadcast(false);
	}
}

void UGS_GameInstance::HandleLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error)
{
	if (IdentityInterface.IsValid())
	{
		IdentityInterface->ClearOnLoginCompleteDelegate_Handle(LocalUserNum, LoginCompleteDelegateHandle);
	}

	if (!bWasSuccessful)
	{
		UE_LOG(LogTemp, Warning, TEXT("GS_GameInstance Login failed: %s"), *Error);
	}

	OnGSLoginComplete.Broadcast(bWasSuccessful);
}

void UGS_GameInstance::RequestFriendsList()
{
	if (!FriendsInterface.IsValid())
	{
		OnGSFriendsListComplete.Broadcast(false);
		return;
	}

	FriendsInterface->ReadFriendsList(0, EFriendsLists::ToString(EFriendsLists::Default),
		FOnReadFriendsListComplete::CreateUObject(this, &UGS_GameInstance::HandleReadFriendsListComplete));
}

void UGS_GameInstance::HandleReadFriendsListComplete(int32 LocalUserNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr)
{
	CachedFriends.Empty();

	if (bWasSuccessful && FriendsInterface.IsValid())
	{
		FriendsInterface->GetFriendsList(LocalUserNum, ListName, CachedFriends);

		UE_LOG(LogTemp, Log, TEXT("[EOS] Friends list loaded: %d friend(s)"), CachedFriends.Num());
		for (int32 Index = 0; Index < CachedFriends.Num(); ++Index)
		{
			UE_LOG(LogTemp, Log, TEXT("[EOS]   [%d] %s (Presence: %s)"),
				Index, *CachedFriends[Index]->GetDisplayName(),
				*CachedFriends[Index]->GetPresence().Status.StatusStr);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("GS_GameInstance ReadFriendsList failed: %s"), *ErrorStr);
	}

	OnGSFriendsListComplete.Broadcast(bWasSuccessful);
}

void UGS_GameInstance::GetCachedFriends(TArray<FGSFriendInfo>& OutFriends) const
{
	OutFriends.Empty();

	for (int32 Index = 0; Index < CachedFriends.Num(); ++Index)
	{
		FGSFriendInfo Info;
		Info.DisplayName = CachedFriends[Index]->GetDisplayName();
		Info.FriendIndex = Index;
		OutFriends.Add(Info);
	}
}

void UGS_GameInstance::HostParty(int32 MaxPlayers, FName LobbyLevelName)
{
	if (!SessionInterface.IsValid())
	{
		OnGSCreateSessionComplete.Broadcast(false);
		return;
	}

	PendingLobbyLevelName = LobbyLevelName;

	FOnlineSessionSettings SessionSettings;
	SessionSettings.bIsLANMatch = false;
	SessionSettings.NumPublicConnections = MaxPlayers;
	SessionSettings.bShouldAdvertise = true;
	SessionSettings.bUsesPresence = true;
	SessionSettings.bAllowJoinInProgress = true;
	SessionSettings.bAllowJoinViaPresence = true;
	SessionSettings.bAllowInvites = true;
	SessionSettings.bUseLobbiesIfAvailable = true;

	CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
		FOnCreateSessionCompleteDelegate::CreateUObject(this, &UGS_GameInstance::HandleCreateSessionComplete));

	if (!SessionInterface->CreateSession(0, NAME_GameSession, SessionSettings))
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
		OnGSCreateSessionComplete.Broadcast(false);
	}
}

void UGS_GameInstance::HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	}

	UE_LOG(LogTemp, Log, TEXT("[EOS] CreateSession(%s) complete -> %s"),
		*SessionName.ToString(), bWasSuccessful ? TEXT("success") : TEXT("failed"));

	if (bWasSuccessful && !PendingLobbyLevelName.IsNone())
	{
		UGameplayStatics::OpenLevel(this, PendingLobbyLevelName, true, TEXT("listen"));
	}

	OnGSCreateSessionComplete.Broadcast(bWasSuccessful);
}

void UGS_GameInstance::InviteFriend(int32 FriendIndex)
{
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[EOS] InviteFriend failed: SessionInterface invalid"));
		return;
	}

	if (!CachedFriends.IsValidIndex(FriendIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("[EOS] InviteFriend failed: FriendIndex %d invalid (CachedFriends count = %d)"),
			FriendIndex, CachedFriends.Num());
		return;
	}

	const TSharedRef<FOnlineFriend> Friend = CachedFriends[FriendIndex];
	const bool bSent = SessionInterface->SendSessionInviteToFriend(0, NAME_GameSession, *Friend->GetUserId());

	UE_LOG(LogTemp, Log, TEXT("[EOS] SendSessionInviteToFriend to %s (%s) -> %s"),
		*Friend->GetDisplayName(), *Friend->GetUserId()->ToString(), bSent ? TEXT("true") : TEXT("false"));
}

void UGS_GameInstance::HandleSessionInviteReceived(const FUniqueNetId& UserId, const FUniqueNetId& FromId, const FString& AppId, const FOnlineSessionSearchResult& InviteResult)
{
	PendingInviteResult = MakeShared<FOnlineSessionSearchResult>(InviteResult);

	FString InviterName = FromId.ToString();

	if (FriendsInterface.IsValid())
	{
		TSharedPtr<FOnlineFriend> InviterFriend = FriendsInterface->GetFriend(0, FromId, EFriendsLists::ToString(EFriendsLists::Default));
		if (InviterFriend.IsValid())
		{
			InviterName = InviterFriend->GetDisplayName();
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[EOS] Session invite received from %s"), *InviterName);

	OnGSInviteReceived.Broadcast(InviterName);
}

void UGS_GameInstance::HandleSessionUserInviteAccepted(const bool bWasSuccessful, const int32 ControllerId, FUniqueNetIdPtr UserId, const FOnlineSessionSearchResult& InviteResult)
{
	if (!bWasSuccessful || !SessionInterface.IsValid())
	{
		OnGSJoinSessionComplete.Broadcast(false);
		return;
	}

	PendingInviteResult = MakeShared<FOnlineSessionSearchResult>(InviteResult);

	JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
		FOnJoinSessionCompleteDelegate::CreateUObject(this, &UGS_GameInstance::HandleJoinSessionComplete));

	if (!SessionInterface->JoinSession(0, NAME_GameSession, InviteResult))
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
		OnGSJoinSessionComplete.Broadcast(false);
	}
}

void UGS_GameInstance::AcceptPendingInvite()
{
	if (!SessionInterface.IsValid() || !PendingInviteResult.IsValid())
	{
		OnGSJoinSessionComplete.Broadcast(false);
		return;
	}

	JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
		FOnJoinSessionCompleteDelegate::CreateUObject(this, &UGS_GameInstance::HandleJoinSessionComplete));

	if (!SessionInterface->JoinSession(0, NAME_GameSession, *PendingInviteResult))
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
		OnGSJoinSessionComplete.Broadcast(false);
	}
}

void UGS_GameInstance::HandleJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
	}

	const bool bSuccess = (Result == EOnJoinSessionCompleteResult::Success);

	if (bSuccess && SessionInterface.IsValid())
	{
		FString ConnectString;
		if (SessionInterface->GetResolvedConnectString(SessionName, ConnectString))
		{
			if (APlayerController* PC = GetFirstLocalPlayerController())
			{
				PC->ClientTravel(ConnectString, TRAVEL_Absolute);
			}
		}
	}

	OnGSJoinSessionComplete.Broadcast(bSuccess);
}

void UGS_GameInstance::StartGame(FName GameLevelName)
{
	UWorld* World = GetWorld();
	if (!World || World->GetNetMode() == NM_Client)
	{
		return;
	}

	World->ServerTravel(GameLevelName.ToString() + TEXT("?listen"), true);
}
