#include "GS_HUDWidget.h"

#include "AbilitySystemComponent.h"
#include "Components/PanelWidget.h"
#include "Gang_Squirrel/Game/GS_GameState.h"
#include "Gang_Squirrel/GAS/AttributeSet/GS_PlayerAttributeSet.h"
#include "Gang_Squirrel/Player/GS_PlayerState.h"
#include "Gang_Squirrel/UI/Party/GS_PartyWidget.h"
#include "Gang_Squirrel/UI/Stat_Widget/GS_HPCountWidget.h"

void UGS_HUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindToMyPlayerState();
	
	GetWorld()->GetTimerManager().SetTimer(MyStateBindTimerHandle,this,&UGS_HUDWidget::PollForPlayer,0.2f,true);
	
	if (PartyWidgetClass)
	{
		UGS_PartyWidget* PartyWidget = CreateWidget<UGS_PartyWidget>(this,PartyWidgetClass);
		if (PartyWidget)
		{
			PartyWidget->AddToViewport();
		}
	}
}

void UGS_HUDWidget::NativeDestruct()
{
	GetWorld()->GetTimerManager().ClearTimer(MyStateBindTimerHandle);
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

void UGS_HUDWidget::PollForPlayer()
{
	if (!bMyStateBound)
	{
		BindToMyPlayerState();
	}
	if (bMyStateBound)
	{
		GetWorld()->GetTimerManager().ClearTimer(MyStateBindTimerHandle);
	}
}

void UGS_HUDWidget::OnMyHealthChanged(const FOnAttributeChangeData& Data)
{
	RefreshMyHealth();
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
