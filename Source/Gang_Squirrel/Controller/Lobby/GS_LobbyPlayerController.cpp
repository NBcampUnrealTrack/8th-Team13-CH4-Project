#include "GS_LobbyPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "Gang_Squirrel/Character/GSCharacter.h"
#include "Gang_Squirrel/EOS/GS_GameInstance.h"
#include "Gang_Squirrel/EOS/LobbySettings/GS_LobbyCharacterSlot.h"
#include "Gang_Squirrel/Game/GS_GameState.h"
#include "Gang_Squirrel/Game/Lobby/GS_LobbyGameMode.h"
#include "Gang_Squirrel/Player/GS_PlayerState.h"
#include "Kismet/GameplayStatics.h"

void AGS_LobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	if (!IsLocalController())
	{
		return;
	}
	
	if (LobbyWidgetClass)
	{
		UUserWidget* LobbyWidget = CreateWidget<UUserWidget>(this, LobbyWidgetClass);
		if (LobbyWidget)
		{
			LobbyWidget->AddToViewport();
			SetShowMouseCursor(true);
			SetInputMode(FInputModeUIOnly());
		}
	}
	
	SetupLobbyCamera();
	CacheCharacterSlots();
	
	FString DisplayName;
	if (UGS_GameInstance* GSInst = GetGameInstance<UGS_GameInstance>())
	{
		DisplayName = GSInst->GetLocalDisplayName();
	}
	if (DisplayName.IsEmpty())
	{
		DisplayName = FString::Printf(TEXT("Player_%d"),GetLocalPlayer() ? GetLocalPlayer()->GetControllerId() : 0);
	}
	ServerSetNickname(DisplayName);
	
	GetWorldTimerManager().SetTimer(CharacterDisplayTimerHandle, this, &AGS_LobbyPlayerController::RefreshCharacterDisplay,0.5f,true,0.f);
	
	
	if (GetNetMode() == NM_Client)
	{
		return;
	}
	
	UGS_GameInstance* GSInst = GetGameInstance<UGS_GameInstance>();
	
	if (!GSInst)
	{
		return;
	}
	
	if (GSInst->IsLoggedIn())
	{
		GSInst->HostParty(MaxPartyPlayers,NAME_None);
	}
	else
	{
		GSInst->OnGSLoginComplete.AddDynamic(this, &AGS_LobbyPlayerController::OnLoginCompleteForHost);
	}
	
	
}

void AGS_LobbyPlayerController::RequestStartGame()
{
	ServerRequestStartGame();
}

void AGS_LobbyPlayerController::ServerRequestStartGame_Implementation()
{
	if (AGS_LobbyGameMode* GM = GetWorld()->GetAuthGameMode<AGS_LobbyGameMode>())
	{
		GM->TryStartGame(this);
	}
}

void AGS_LobbyPlayerController::OnLoginCompleteForHost(bool bWasSuccessful)
{
	UGS_GameInstance* GSInst = GetGameInstance<UGS_GameInstance>();
	if (!GSInst)
	{
		return;
	}
	
	GSInst->OnGSLoginComplete.RemoveDynamic(this, &AGS_LobbyPlayerController::OnLoginCompleteForHost);
	
	if (bWasSuccessful)
	{
		GSInst->HostParty(MaxPartyPlayers,NAME_None);;
	}
}

void AGS_LobbyPlayerController::ServerSetNickname_Implementation(const FString& Nickname)
{
	AGS_PlayerState* PS = GetPlayerState<AGS_PlayerState>();
	if (!PS || !PS->PlayerNickname.IsEmpty())
	{
		return;
	}
	PS->SetPlayerNickname(Nickname);
}

void AGS_LobbyPlayerController::SetupLobbyCamera()
{
	TArray<AActor*> FoundCamera;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(),LobbyCameraTag,FoundCamera);
	
	if (FoundCamera.Num() > 0)
	{
		SetViewTargetWithBlend(FoundCamera[0],0.f);
	}
}

void AGS_LobbyPlayerController::CacheCharacterSlots()
{
	TArray<AActor*> FoundSlots;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(),AGS_LobbyCharacterSlot::StaticClass(),FoundSlots);
	
	FoundSlots.Sort([](const AActor& A, const AActor& B)
	{
		return Cast<AGS_LobbyCharacterSlot>(&A)->SlotIndex < Cast<AGS_LobbyCharacterSlot>(&B)->SlotIndex;
	});
	
	SlotTransforms.Reset();
	for (AActor* Slot : FoundSlots)
	{
		SlotTransforms.Add(Slot->GetActorTransform());
	}
}

void AGS_LobbyPlayerController::RefreshCharacterDisplay()
{
	if (!DisplayCharacterClass)
	{
		return;
	}
	
	AGS_GameState* GS = GetWorld()->GetGameState<AGS_GameState>();
	if (!GS)
	{
		return;
	}
	
	int32 SlotIndex = 0;
	for (APlayerState* PS : GS->PlayerArray)
	{
		AGS_PlayerState* CandidatePS = Cast<AGS_PlayerState>(PS);
		if (!CandidatePS || SlotIndex >= SlotTransforms.Num())
		{
			continue;
		}
		
		if (!DisplayCharacters.IsValidIndex(SlotIndex) || !IsValid(DisplayCharacters[SlotIndex]))
		{
			AGSCharacter* NewCharacter = GetWorld()->SpawnActor<AGSCharacter>(DisplayCharacterClass, SlotTransforms[SlotIndex]);
			if (!DisplayCharacters.IsValidIndex(SlotIndex))
			{
				DisplayCharacters.SetNum(SlotIndex + 1);
			}
			DisplayCharacters[SlotIndex] = NewCharacter;
		}
		
		if (IsValid(DisplayCharacters[SlotIndex]))
		{
			DisplayCharacters[SlotIndex]->UpdateNameTag(CandidatePS->PlayerNickname);
		}
		
		++SlotIndex;
	}
	
	for (int32 i = SlotIndex; i < DisplayCharacters.Num(); ++i)
	{
		if (IsValid(DisplayCharacters[i]))
		{
			DisplayCharacters[i]->Destroy();
		}
		DisplayCharacters[i] = nullptr;
	}
}
