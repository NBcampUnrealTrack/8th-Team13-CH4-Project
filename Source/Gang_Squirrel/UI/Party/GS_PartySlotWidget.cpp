#include "GS_PartySlotWidget.h"

#include "AbilitySystemComponent.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Gang_Squirrel/GAS/AttributeSet/GS_PlayerAttributeSet.h"
#include "Gang_Squirrel/Player/GS_PlayerState.h"
#include "Gang_Squirrel/UI/Stat_Widget/GS_HPCountWidget.h"

void UGS_PartySlotWidget::InitSlot(AGS_PlayerState* InPlayerState, int32 InSlotIndex)
{
	if (BoundPlayerState == InPlayerState)
	{
		return;
	}
	
	if (BoundPlayerState)
	{
		BoundPlayerState->OnPlayerScoreChanged.RemoveDynamic(this,&UGS_PartySlotWidget::OnScoreChanged);
		BoundPlayerState->OnPlayerNameChanged.RemoveDynamic(this,&UGS_PartySlotWidget::OnNicknameChanged);
		
		if (UAbilitySystemComponent* PrevASC = BoundPlayerState->GetAbilitySystemComponent())
		{
			PrevASC->GetGameplayAttributeValueChangeDelegate(UGS_PlayerAttributeSet::GetHealthAttribute()).RemoveAll(this);
			PrevASC->GetGameplayAttributeValueChangeDelegate(UGS_PlayerAttributeSet::GetMaxHealthAttribute()).RemoveAll(this);
		}
	}
	
	BoundPlayerState = InPlayerState;
	if (!BoundPlayerState)
	{
		return;
	}
	
	if (Text_Portrait)
	{
		Text_Portrait->SetText(FText::FromString(FString::Printf(TEXT("P%d"),InSlotIndex + 1)));
	}
	
	if (Border_Portrait && SlotsColors.IsValidIndex(InSlotIndex))
	{
		Border_Portrait->SetBrushColor(SlotsColors[InSlotIndex]);
	}
	
	BoundPlayerState->OnPlayerNameChanged.AddDynamic(this,&UGS_PartySlotWidget::OnNicknameChanged);
	BoundPlayerState->OnPlayerScoreChanged.AddDynamic(this,&UGS_PartySlotWidget::OnScoreChanged);
	
	if (UAbilitySystemComponent* ASC = BoundPlayerState->GetAbilitySystemComponent())
	{
		ASC->GetGameplayAttributeValueChangeDelegate(UGS_PlayerAttributeSet::GetHealthAttribute()).AddUObject(this,&UGS_PartySlotWidget::OnHealthChanged);
		ASC->GetGameplayAttributeValueChangeDelegate(UGS_PlayerAttributeSet::GetMaxHealthAttribute()).AddUObject(this,&UGS_PartySlotWidget::OnHealthChanged);
	}
	
	RefreshNickName();
	
	if (Text_Score)
	{
		Text_Score->SetText(FText::AsNumber(BoundPlayerState->GetPlayerScore()));
	}
	
	RefreshHealth();
}

void UGS_PartySlotWidget::NativeDestruct()
{
	Super::NativeDestruct();
}

void UGS_PartySlotWidget::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	RefreshHealth();
}

void UGS_PartySlotWidget::OnNicknameChanged(const FString& NewName)
{
	RefreshNickName();
}

void UGS_PartySlotWidget::OnScoreChanged(int32 NewScore)
{
	if (Text_Score)
	{
		Text_Score->SetText(FText::AsNumber(NewScore));
	}
}

void UGS_PartySlotWidget::RefreshHealth()
{
	UAbilitySystemComponent* ASC = BoundPlayerState ? BoundPlayerState->GetAbilitySystemComponent() : nullptr;
	if (!ASC || !HPWidget)
	{
		return;
	}
	HPWidget->SetHealth(FMath::RoundToInt(ASC->GetNumericAttribute(UGS_PlayerAttributeSet::GetHealthAttribute())),
		FMath::RoundToInt(ASC->GetNumericAttribute(UGS_PlayerAttributeSet::GetMaxHealthAttribute())));
}

void UGS_PartySlotWidget::RefreshNickName()
{
	if (Text_NickName && BoundPlayerState)
	{
		Text_NickName->SetText(FText::FromString(BoundPlayerState->PlayerNickname));
	}
}
