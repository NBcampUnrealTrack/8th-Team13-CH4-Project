// Fill out your copyright notice in the Description page of Project Settings.


#include "GS_GameModeBase.h"
#include "Gang_Squirrel/Game/GS_GameState.h"
#include "EngineUtils.h"
#include "AbilitySystemComponent.h"
#include "Gang_Squirrel/Gimmick/GS_FallingHazardManager.h"
#include "Gang_Squirrel/SpawnSystem/GSSpawnManager.h"
#include "Gang_Squirrel/Player/GS_PlayerState.h"
#include "Gang_Squirrel/DataAsset/GSFoodPrimaryDataAsset.h"
#include "Gang_Squirrel/Character/GSCharacter.h"
#include "Gang_Squirrel/GAS/AttributeSet/GS_PlayerAttributeSet.h"
#include "Gang_Squirrel/EOS/GS_GameInstance.h"
#include "Gang_Squirrel/GameObjects/GS_PlayerStart.h"
#include "Gang_Squirrel/SpawnSystem/Enemy/GS_EnemySpawnManager.h"

AGS_GameModeBase::AGS_GameModeBase()
{
	MatchTimeLimit = 600.f;
	bUseSeamlessTravel = true;
}

void AGS_GameModeBase::BeginPlay()
{
	Super::BeginPlay();

	//StartMatch();

	SpawnSpawnManager();
}

void AGS_GameModeBase::StartMatch()
{
	bMatchEnd = false;

	AGS_GameState* GS = GetGameState<AGS_GameState>();
	if (GS)
	{
		GS->MatchEndTime = GetWorld()->GetTimeSeconds() + MatchTimeLimit;
	}
	
		GetWorldTimerManager().SetTimer(
			MatchTimerHandle,
			this,
			&AGS_GameModeBase::OnMatchTimeExpired,
			MatchTimeLimit,
			false
		);

		for (TActorIterator<AGS_FallingHazardManager> It(GetWorld()); It; ++It)
		{
			AGS_FallingHazardManager* HazardManager = *It;
			if (IsValid(HazardManager))
			{
				HazardManager->StartSpawnFallingHazard();
			}
		}
	
	//UE_LOG(LogTemp, Log, TEXT("[Server] Match Started. %.1f sec"), MatchTimeLimit)
	
	// SpawnEnemy
	for (TActorIterator<AGS_EnemySpawnManager> It(GetWorld()); It; ++It)
	{
		AGS_EnemySpawnManager* EnemySpawnManager = *It;
		if (IsValid(EnemySpawnManager))
		{
			EnemySpawnManager->StartSpawnEnemy();
		}
	}
}

void AGS_GameModeBase::EndMatch()
{
	if (bMatchEnd)
	{
		return;
	}

	bMatchEnd = true;
	GetWorldTimerManager().ClearTimer(MatchTimerHandle);


	for (TActorIterator<AGS_FallingHazardManager> It(GetWorld()); It; ++It)
	{
		AGS_FallingHazardManager* HazardManager = *It;
		if (IsValid(HazardManager))
		{
			HazardManager->StopSpawnFallingHazard();
		}
	}
	
	// SpawnEnemy
	for (TActorIterator<AGS_EnemySpawnManager> It(GetWorld()); It; ++It)
	{
		AGS_EnemySpawnManager* EnemySpawnManager = *It;
		if (IsValid(EnemySpawnManager))
		{
			EnemySpawnManager->StopSpawnEnemy();
		}
	}

	//UE_LOG(LogTemp, Log, TEXT("[Server] Match Ended."))

	// 모든 플레이어를 Result 레벨로 이동시킴
	if (UGS_GameInstance* GSInst = GetGameInstance<UGS_GameInstance>())
	{
		GSInst->StartGame(ResultLevelName);
	}
}

void AGS_GameModeBase::NotifyPlayerReady()
{
	ReadyPlayerCount++;

	MultiCastRPCPrintStatus(ReadyPlayerCount, GetNumPlayers());

	// UE_LOG(LogTemp, Warning, TEXT("[Server] Player ready : %d / %d"), ReadyPlayerCount, GetNumPlayers());


	//Match started when all players input nickname
	if (ReadyPlayerCount >= GetNumPlayers())
	{
		StartMatch();
	}
}

void AGS_GameModeBase::GiveRandomReward(AGS_PlayerState* KillerPS)
{
	if (!HasAuthority() || !IsValid(KillerPS)) return;

	const ERewardType RewardType = static_cast<ERewardType>(FMath::RandRange(0, 2));
	GiveSpecificReward(KillerPS, RewardType);
	
}

void AGS_GameModeBase::GiveSpecificReward(AGS_PlayerState* KillerPS, ERewardType RewardType)
{
	if (!HasAuthority() || !IsValid(KillerPS)) return;

	AGSCharacter* Character = Cast<AGSCharacter>(KillerPS->GetPawn());
	
	if (!Character) return;
	
	switch (RewardType)
	{
	case ERewardType::Food:
		GiveFoodReward(KillerPS);
		Character->UpdateSlideWidget(0);
		break;
	case ERewardType::Capacity:
		GiveCapacityReward(KillerPS);
		Character->UpdateSlideWidget(1);
		break;
	case ERewardType::SpeedBoost:
		GiveSpeedBoostReward(KillerPS);
		Character->UpdateSlideWidget(2);
		break;
	}
}

void AGS_GameModeBase::OnMatchTimeExpired()
{
	EndMatch();
}

void AGS_GameModeBase::MultiCastRPCPrintStatus_Implementation(int32 Ready, int32 Total)
{
	// (LogTemp, Log, TEXT("%d / %d 준비완료!!"), Ready, Total)
}

void AGS_GameModeBase::SpawnSpawnManager() const
{
	if (!IsValid(SpawnManagerClass)) return;
	AGSSpawnManager* SpawnManager = GetWorld()->SpawnActor<AGSSpawnManager>(SpawnManagerClass);
}

void AGS_GameModeBase::GiveFoodReward(AGS_PlayerState* PS)
{
	if (!IsValid(RewardFoodData)) return;

	PS->AddScore(RewardFoodData->ScoreAmount);

	UE_LOG(LogTemp, Log, TEXT("[Reward] Food Reward: +%d"), RewardFoodData->ScoreAmount);
}

void AGS_GameModeBase::GiveCapacityReward(AGS_PlayerState* PS)
{
	if (IsValid(PS) == false)
	{
		return;
	}

	AGSCharacter* Character = Cast<AGSCharacter>(PS->GetPawn());
	if (IsValid(Character) == false)
	{
		return;
	}

	Character->AddMaxCheekSize(CapacityIncreaseAmount);
	UE_LOG(LogTemp, Log, TEXT("[Reward] Capacity Reward: +%.2f"), CapacityIncreaseAmount);
}

void AGS_GameModeBase::GiveSpeedBoostReward(AGS_PlayerState* PS)
{
	if (IsValid(PS) == false)
	{
		return;
	}

	UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
	if (IsValid(ASC) == false)
	{
		return;
	}

	if (!IsValid(GE_MoveSpeedRewardClass))
	{
		// UE_LOG(LogTemp, Warning, TEXT("[Reward] GE_MoveSpeedRewardClass is not assigned in GameMode BP"));
		return;
	}

	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(GE_MoveSpeedRewardClass, 1.f, Context);

	if (SpecHandle.IsValid())
	{
		ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

		UE_LOG(LogTemp, Log, TEXT("[Reward] MoveSpeed Reward Applied. Current MoveSpeed: %.1f"),
			ASC->GetNumericAttribute(UGS_PlayerAttributeSet::GetMoveSpeedAttribute()));
	}

}

AActor* AGS_GameModeBase::ChoosePlayerStart_Implementation(AController* Player)
{
	if (const AGS_PlayerState* PS = Player ? Player->GetPlayerState<AGS_PlayerState>() : nullptr)
	{
		if (PS->GetLobbySlotIndex() >= 0)
		{
			for (TActorIterator<AGS_PlayerStart> It(GetWorld()); It; ++It)
			{
				if (It->SlotIndex == PS->GetLobbySlotIndex())
				{
					return *It;
				}
			}
		}
	}
	
	return Super::ChoosePlayerStart_Implementation(Player);
}
