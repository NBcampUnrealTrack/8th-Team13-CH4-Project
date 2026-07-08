#include "GS_HUDWidget.h"

#include "AbilitySystemComponent.h"
#include "Gang_Squirrel/Game/GS_GameState.h"
#include "Gang_Squirrel/GAS/AttributeSet/GS_PlayerAttributeSet.h"
#include "Gang_Squirrel/Player/GS_PlayerState.h"
#include "Gang_Squirrel/UI/Stat_Widget/GS_HPCountWidget.h"

void UGS_HUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindToMyPlayerState();
	
	GetWorld()->GetTimerManager().SetTimer(OtherPlayerFindTimerHandle,this,&UGS_HUDWidget::PollForPlayer,0.2f,true);
}

void UGS_HUDWidget::NativeDestruct()
{
	GetWorld()->GetTimerManager().ClearTimer(OtherPlayerFindTimerHandle);
	Super::NativeDestruct();
}

void UGS_HUDWidget::BindToMyPlayerState()
{
	AGS_PlayerState* MyPS = GetOwningPlayerState<AGS_PlayerState>();
	UAbilitySystemComponent* MyASC = MyPS ? MyPS->GetAbilitySystemComponent() : nullptr;
	if (!MyASC)
	{
		return;
	}
	
	MyASC->GetGameplayAttributeValueChangeDelegate(UGS_PlayerAttributeSet::GetHealthAttribute()).AddUObject(this,&UGS_HUDWidget::OnMyHealthChanged);
	MyASC->GetGameplayAttributeValueChangeDelegate(UGS_PlayerAttributeSet::GetMaxHealthAttribute()).AddUObject(this,&UGS_HUDWidget::OnMyHealthChanged);
	
	RefreshMyHealth();
	bMyStateBound = true;
}

void UGS_HUDWidget::TryFindAndBindOtherPlayer()
{
	AGS_GameState* GS = GetWorld()->GetGameState<AGS_GameState>();
	AGS_PlayerState* MyPS = GetOwningPlayerState<AGS_PlayerState>();
	if (!GS || !MyPS)
	{
		return;
	}
	
	for (APlayerState* PS : GS->PlayerArray)
	{
		AGS_PlayerState* CandidatePS = Cast<AGS_PlayerState>(PS);
		if (CandidatePS && CandidatePS != MyPS)
		{
			OtherPlayerState = CandidatePS;
			break;
		}
	}
	
	if (!OtherPlayerState)
	{
		return;
	}
	
	if (UAbilitySystemComponent* OtherASC = OtherPlayerState->GetAbilitySystemComponent())
	{
		OtherASC->GetGameplayAttributeValueChangeDelegate(UGS_PlayerAttributeSet::GetHealthAttribute()).AddUObject(this,&UGS_HUDWidget::OnOtherHealthChanged);
		OtherASC->GetGameplayAttributeValueChangeDelegate(UGS_PlayerAttributeSet::GetMaxHealthAttribute()).AddUObject(this,&UGS_HUDWidget::OnOtherHealthChanged);
		
		RefreshOtherHealth();
	}
	
	GetWorld()->GetTimerManager().ClearTimer(OtherPlayerFindTimerHandle);
}

void UGS_HUDWidget::PollForPlayer()
{
	if (!bMyStateBound)
	{
		BindToMyPlayerState();
	}
	if (!OtherPlayerState)
	{
		TryFindAndBindOtherPlayer();
	}
	if (bMyStateBound && OtherPlayerState)
	{
		GetWorld()->GetTimerManager().ClearTimer(OtherPlayerFindTimerHandle);
	}
}

void UGS_HUDWidget::OnMyHealthChanged(const FOnAttributeChangeData& Data)
{
	RefreshMyHealth();
}

void UGS_HUDWidget::OnOtherHealthChanged(const FOnAttributeChangeData& Data)
{
	RefreshOtherHealth();
}

void UGS_HUDWidget::RefreshMyHealth()
{
	AGS_PlayerState* MyPS = GetOwningPlayerState<AGS_PlayerState>();
	UAbilitySystemComponent* MyASC = MyPS ? MyPS->GetAbilitySystemComponent() : nullptr;
	
	if (!MyASC || !HPWidget_Mine)
	{
		return;
	}
	
	HPWidget_Mine->SetHealth(FMath::RoundToInt(MyASC->GetNumericAttribute(UGS_PlayerAttributeSet::GetHealthAttribute()))
		,FMath::RoundToInt(MyASC->GetNumericAttribute(UGS_PlayerAttributeSet::GetMaxHealthAttribute())));
}

void UGS_HUDWidget::RefreshOtherHealth()
{
	UAbilitySystemComponent* OtherASC = OtherPlayerState ? OtherPlayerState->GetAbilitySystemComponent() : nullptr;
	if (!OtherASC || !HPWidget_Other)
	{
		return;
	}
	
	HPWidget_Other->SetHealth(FMath::RoundToInt(OtherASC->GetNumericAttribute(UGS_PlayerAttributeSet::GetHealthAttribute()))
		,FMath::RoundToInt(OtherASC->GetNumericAttribute(UGS_PlayerAttributeSet::GetMaxHealthAttribute())));
}
