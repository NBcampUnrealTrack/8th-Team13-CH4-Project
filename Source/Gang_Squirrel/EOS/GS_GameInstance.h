#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Interfaces/OnlineFriendsInterface.h"
#include "GS_GameInstance.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGSLoginComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGSFriendsListComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGSInviteReceived, FString, InviterDisplayName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGSCreateSessionComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGSJoinSessionComplete, bool, bWasSuccessful);

USTRUCT(BlueprintType)
struct FGSFriendInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "EOS")
	FString DisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "EOS")
	int32 FriendIndex = -1;
};

UCLASS()
class GANG_SQUIRREL_API UGS_GameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;
	virtual void Shutdown() override;

	// 로그인
	UFUNCTION(BlueprintCallable, Category = "EOS")
	void Login();

	// 친구 목록 갱신 요청 (완료되면 OnGSFriendsListComplete 발생)
	UFUNCTION(BlueprintCallable, Category = "EOS")
	void RequestFriendsList();

	// 갱신된 친구 목록 가져오기 (RequestFriendsList 완료 후 호출)
	UFUNCTION(BlueprintCallable, Category = "EOS")
	void GetCachedFriends(TArray<FGSFriendInfo>& OutFriends) const;

	// 파티 호스트: 세션 생성 + 성공 시 LobbyLevelName을 리슨 서버로 오픈
	UFUNCTION(BlueprintCallable, Category = "EOS")
	void HostParty(int32 MaxPlayers, FName LobbyLevelName);

	// 친구에게 파티 초대 전송 (호스트가 호출)
	UFUNCTION(BlueprintCallable, Category = "EOS")
	void InviteFriend(int32 FriendIndex);

	// 받은 초대 수락 (클라이언트가 호출) -> 성공 시 호스트 로비로 자동 접속
	UFUNCTION(BlueprintCallable, Category = "EOS")
	void AcceptPendingInvite();

	// 호스트가 게임 시작 -> 접속된 모두 Seamless Travel로 같이 이동 (호스트 전용)
	UFUNCTION(BlueprintCallable, Category = "EOS")
	void StartGame(FName GameLevelName);

	UPROPERTY(BlueprintAssignable)
	FOnGSLoginComplete OnGSLoginComplete;

	UPROPERTY(BlueprintAssignable)
	FOnGSFriendsListComplete OnGSFriendsListComplete;

	UPROPERTY(BlueprintAssignable)
	FOnGSInviteReceived OnGSInviteReceived;

	UPROPERTY(BlueprintAssignable)
	FOnGSCreateSessionComplete OnGSCreateSessionComplete;

	UPROPERTY(BlueprintAssignable)
	FOnGSJoinSessionComplete OnGSJoinSessionComplete;

private:
	IOnlineSessionPtr SessionInterface;
	IOnlineIdentityPtr IdentityInterface;
	IOnlineFriendsPtr FriendsInterface;

	TArray<TSharedRef<FOnlineFriend>> CachedFriends;
	TSharedPtr<FOnlineSessionSearchResult> PendingInviteResult;
	FName PendingLobbyLevelName;

	FDelegateHandle LoginCompleteDelegateHandle;
	FDelegateHandle CreateSessionCompleteDelegateHandle;
	FDelegateHandle JoinSessionCompleteDelegateHandle;
	FDelegateHandle SessionInviteReceivedDelegateHandle;
	FDelegateHandle SessionUserInviteAcceptedDelegateHandle;

	void HandleLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error);
	void HandleReadFriendsListComplete(int32 LocalUserNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr);
	void HandleSessionInviteReceived(const FUniqueNetId& UserId, const FUniqueNetId& FromId, const FString& AppId, const FOnlineSessionSearchResult& InviteResult);
	void HandleSessionUserInviteAccepted(const bool bWasSuccessful, const int32 ControllerId, FUniqueNetIdPtr UserId, const FOnlineSessionSearchResult& InviteResult);
	void HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void HandleJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
};
