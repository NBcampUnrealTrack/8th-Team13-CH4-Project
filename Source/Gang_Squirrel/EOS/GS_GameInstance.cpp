#include "GS_GameInstance.h"

#include "Online.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlinePresenceInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/PostProcessVolume.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"
#include "MoviePlayer.h"
#include "UObject/UObjectGlobals.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "Gang_Squirrel/UI/Lobby/GS_SettingWidget.h"

void UGS_GameInstance::Init()
{
	Super::Init();

	//Loading
	FCoreUObjectDelegates::PreLoadMap.AddUObject(
		this,
		&UGS_GameInstance::BeginLoadingScreen
	);

	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(
		this,
		&UGS_GameInstance::EndLoadingScreen
	);

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
	
#if WITH_EDITOR
		AutoLoginForPIETest();
#endif
	
	if (MasterSoundMix)
	{
		UGameplayStatics::PushSoundMixModifier(this, MasterSoundMix);
	}
}

void UGS_GameInstance::Shutdown()
{
	FCoreUObjectDelegates::PreLoadMap.RemoveAll(this);
	FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);

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

#if WITH_EDITOR
void UGS_GameInstance::AutoLoginForPIETest()
{
	if (!IdentityInterface.IsValid() || IdentityInterface->GetLoginStatus(0) == ELoginStatus::LoggedIn)
	{
		return;
	}
	
	const int32 InstanceIndex = GetPIEInstanceIndexFromCommandLine();
	const FString CredentialName = (InstanceIndex == 0) ? TEXT("dev1") : TEXT("dev2");
	
	LoginCompleteDelegateHandle = IdentityInterface->AddOnLoginCompleteDelegate_Handle(
		0,FOnLoginCompleteDelegate::CreateUObject(this,&UGS_GameInstance::HandleLoginComplete));
	
	const FOnlineAccountCredentials Credentials(TEXT("developer"),TEXT("localhost:6300"),CredentialName);
	if (!IdentityInterface->Login(0, Credentials))
	{
		IdentityInterface->ClearOnLoginCompleteDelegate_Handle(0, LoginCompleteDelegateHandle);
	}
	
	UE_LOG(LogTemp, Log, TEXT("[EOS] PIE auto-login as '%s' (instance %d)"), *CredentialName, InstanceIndex);
}
#endif

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

	const FOnlineAccountCredentials Credentials(TEXT("accountportal"), TEXT(""), TEXT(""));
	if (!IdentityInterface->Login(0, Credentials))
	{
		IdentityInterface->ClearOnLoginCompleteDelegate_Handle(0, LoginCompleteDelegateHandle);
		OnGSLoginComplete.Broadcast(false);
	}
}

bool UGS_GameInstance::IsLoggedIn() const
{
	return IdentityInterface.IsValid() && IdentityInterface->GetLoginStatus(0) == ELoginStatus::LoggedIn;
}

FString UGS_GameInstance::GetLocalDisplayName() const
{
	if (!IdentityInterface.IsValid() || IdentityInterface->GetLoginStatus(0) != ELoginStatus::LoggedIn)
	{
		return FString();
	}
	
	return IdentityInterface->GetPlayerNickname(0);
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

#if WITH_EDITOR
	if (bWasSuccessful && bWantsListenServerInPIE && GetWorld() && GetWorld()->GetNetMode() != NM_ListenServer)
	{
		const bool bListen = EnableListenServer(true,PendingPIEListenPort);
	}
#endif
	
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

	if (bWasSuccessful)
	{
		if (!PendingLobbyLevelName.IsNone())
		{
			UGameplayStatics::OpenLevel(this, PendingLobbyLevelName, true, TEXT("listen"));
		}
		else if (GetWorld() && GetWorld()->GetNetMode() != NM_ListenServer)
		{
			EnableListenServer(true);
		}
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
	JoinPendingSession();
}

void UGS_GameInstance::AcceptPendingInvite()
{
	if (!SessionInterface.IsValid() || !PendingInviteResult.IsValid())
	{
		OnGSJoinSessionComplete.Broadcast(false);
		return;
	}
	
	JoinPendingSession();
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

#if WITH_EDITOR
int32 UGS_GameInstance::GetPIEInstanceIndexFromCommandLine()
{
	FString Value;
	if (FParse::Value(FCommandLine::Get(), TEXT("GameUserSettingsINI="),Value))
	{
		static const FString Marker = TEXT("PIEGameUserSettings");
		const int32 MarkerPos = Value.Find(Marker);
		if (MarkerPos != INDEX_NONE)
		{
			FString Digits;
			for (int32 i = MarkerPos + Marker.Len(); i < Value.Len() && FChar::IsDigit(Value[i]); ++i)
			{
				Digits.AppendChar(Value[i]);
			}
			if (!Digits.IsEmpty())
			{
				return FCString::Atoi(*Digits);
			}
		}
	}
	return 0;
}
#endif

void UGS_GameInstance::SetMouseSensitivity(float NewValue)
{
	MouseSensitivity = FMath::Clamp(NewValue, 0.f, 100.f);
}

void UGS_GameInstance::SetMasterVolume(float NewValue)
{
	MasterVolume = FMath::Clamp(NewValue, 0.f, 100.f);

	if (MasterSoundMix && MasterSoundClass)
	{
		UGameplayStatics::SetSoundMixClassOverride(this, MasterSoundMix, MasterSoundClass, MasterVolume / 100.f, 1.0f, 0.f, true);
	}
}

void UGS_GameInstance::SetScreenBrightness(float NewValue)
{
	ScreenBrightness = FMath::Clamp(NewValue, 0.f, 100.f);
	ApplyBrightnessToWorld();
}

UGS_SettingWidget* UGS_GameInstance::ToggleSettingsWidget(APlayerController* OwningPC)
{
	if (!IsValid(OwningPC) || !SettingWidgetClass)
	{
		return nullptr;
	}

	// 최초 1회만 생성 + AddToViewport. 이후로는 Visibility만 토글해서
	// NativeConstruct가 재호출(델리게이트 재바인딩)되는 것을 방지한다.
	if (!IsValid(SettingWidgetInstance))
	{
		SettingWidgetInstance = CreateWidget<UGS_SettingWidget>(OwningPC, SettingWidgetClass);
		if (IsValid(SettingWidgetInstance))
		{
			SettingWidgetInstance->AddToViewport(100);
		}
	}

	if (!IsValid(SettingWidgetInstance))
	{
		return nullptr;
	}

	if (SettingWidgetInstance->GetVisibility() == ESlateVisibility::Visible)
	{
		SettingWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
		return nullptr;
	}

	SettingWidgetInstance->SetVisibility(ESlateVisibility::Visible);
	return SettingWidgetInstance;
}

void UGS_GameInstance::ApplyBrightnessToWorld()
{
	UWorld* World = GetWorld();
	if (!World) return;

	TArray<AActor*> FoundVolumes;
	UGameplayStatics::GetAllActorsOfClass(World, APostProcessVolume::StaticClass(), FoundVolumes);

	const float MappedBias = (ScreenBrightness - 100.f) / 50.f;
	//UE_LOG(LogTemp, Warning, TEXT("[Brightness] Found %d volumes, ScreenBrightness=%.1f, MappedBias=%.2f"),FoundVolumes.Num(), ScreenBrightness, MappedBias);

	for (AActor* Actor : FoundVolumes)
	{
		if (APostProcessVolume* PPVolume = Cast<APostProcessVolume>(Actor))
		{
			PPVolume->Settings.bOverride_AutoExposureBias = true;
			PPVolume->Settings.AutoExposureBias = MappedBias;

			//UE_LOG(LogTemp, Warning, TEXT("[Brightness] Applied to %s, BlendWeight=%.2f, Unbound=%s"),*PPVolume->GetName(), PPVolume->BlendWeight, PPVolume->bUnbound ? TEXT("true") : TEXT("false"));
		}
	}
}

#if WITH_EDITOR
FGameInstancePIEResult UGS_GameInstance::StartPlayInEditorGameInstance(ULocalPlayer* LocalPlayer,
	const FGameInstancePIEParameters& Params)
{

	bWantsListenServerInPIE = (Params.NetMode == PIE_ListenServer);

	if (bWantsListenServerInPIE)
	{
		uint16 ServerPort = 0;
		if (Params.EditorPlaySettings)
		{
			Params.EditorPlaySettings->GetServerPort(ServerPort);
		}
		PendingPIEListenPort = ServerPort;

		Login();
	}


	return Super::StartPlayInEditorGameInstance(LocalPlayer, Params);
}
#endif

void UGS_GameInstance::StartGame(FName GameLevelName)
{
	UWorld* World = GetWorld();
	if (!World || World->GetNetMode() == NM_Client)
	{
		return;
	}

	World->ServerTravel(GameLevelName.ToString() + TEXT("?listen"), true);
}

void UGS_GameInstance::JoinPendingSession()
{
	if (!SessionInterface.IsValid() || !PendingInviteResult.IsValid())
	{
		OnGSJoinSessionComplete.Broadcast(false);
		return;
	}
	
	if (SessionInterface->GetNamedSession(NAME_GameSession) != nullptr)
	{
		SessionInterface->DestroySession(NAME_GameSession,FOnDestroySessionCompleteDelegate::CreateUObject(this, &UGS_GameInstance::HandleDestroySessionForJoin));
	}
	else
	{
		DoJoinSession();
	}
}

void UGS_GameInstance::DoJoinSession()
{
	JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
		FOnJoinSessionCompleteDelegate::CreateUObject(this,&UGS_GameInstance::HandleJoinSessionComplete));
	
	if (!SessionInterface->JoinSession(0, NAME_GameSession, *PendingInviteResult))
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
		OnGSJoinSessionComplete.Broadcast(false);
	}
}

void UGS_GameInstance::HandleDestroySessionForJoin(FName SessionName, bool bWasSuccessful)
{
	DoJoinSession();
}

//Loading
void UGS_GameInstance::BeginLoadingScreen(const FString& MapName)
{
#if !UE_SERVER
	if (IsRunningDedicatedServer())
	{
		return;
	}

	const bool bIsMainStage =
		MapName.Contains(TEXT("/Game/ProjectFile/Level/L_Main_Stage")) ||
		MapName.Contains(TEXT("L_Main_Stage"));

	if (!bIsMainStage)
	{
		return;
	}

	IGameMoviePlayer* MoviePlayer = GetMoviePlayer();
	if (!MoviePlayer || !MoviePlayer->IsInitialized())
	{
		return;
	}

	bIsMainStageLoading = true;

	FLoadingScreenAttributes LoadingScreen;
	LoadingScreen.MinimumLoadingScreenDisplayTime = 5.0f;
	LoadingScreen.bAutoCompleteWhenLoadingCompletes = true;
	LoadingScreen.bWaitForManualStop = false;
	LoadingScreen.bMoviesAreSkippable = false;
	LoadingScreen.PlaybackType = MT_Looped;

	LoadingScreen.MoviePaths.Add(TEXT("LoadingDancing"));

	MoviePlayer->SetupLoadingScreen(LoadingScreen);
	MoviePlayer->PlayMovie();

	UE_LOG(
		LogTemp,
		Log,
		TEXT("[LoadingScreen] Started for map: %s"),
		*MapName
	);
#endif
}

void UGS_GameInstance::EndLoadingScreen(UWorld* LoadedWorld)
{
#if !UE_SERVER
	if (!bIsMainStageLoading)
	{
		return;
	}

	bIsMainStageLoading = false;

	UE_LOG(
		LogTemp,
		Log,
		TEXT("[LoadingScreen] Map load completed: %s"),
		LoadedWorld ? *LoadedWorld->GetName() : TEXT("Unknown")
	);
#endif
}