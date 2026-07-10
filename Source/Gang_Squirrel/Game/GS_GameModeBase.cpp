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

void AGS_GameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (bHasTraveledToMainStage)
	{
		return;
	}

	if (GetNumPlayers() >= RequiredPlayerCountToStart)
	{
		bHasTraveledToMainStage = true;
		GetWorld()->ServerTravel(MainStageLevelName.ToString() + TEXT("?listen"), true);
	}
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
	
	UE_LOG(LogTemp, Log, TEXT("[Server] Match Started. %.1f sec"), MatchTimeLimit)
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

	UE_LOG(LogTemp, Log, TEXT("[Server] Match Ended."))
}

void AGS_GameModeBase::NotifyPlayerReady()
{
	ReadyPlayerCount++;

	MultiCastRPCPrintStatus(ReadyPlayerCount, GetNumPlayers());

	UE_LOG(LogTemp, Warning, TEXT("[Server] Player ready : %d / %d"), ReadyPlayerCount, GetNumPlayers());


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

	switch (RewardType)
	{
	case ERewardType::Food:
		GiveFoodReward(KillerPS);
		break;
	case ERewardType::Capacity:
		GiveCapacityReward(KillerPS);
		break;
	case ERewardType::SpeedBoost:
		GiveSpeedBoostReward(KillerPS);
		break;
	}
}

void AGS_GameModeBase::OnMatchTimeExpired()
{
	EndMatch();
}

void AGS_GameModeBase::MultiCastRPCPrintStatus_Implementation(int32 Ready, int32 Total)
{
	UE_LOG(LogTemp, Log, TEXT("%d / %d 준비완료!!"), Ready, Total)
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
		UE_LOG(LogTemp, Warning, TEXT("[Reward] GE_MoveSpeedRewardClass is not assigned in GameMode BP"));
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
