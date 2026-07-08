#include "GS_HPCountWidget.h"

#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"

void UGS_HPCountWidget::SetHealth(int32 NewCurrentHP, int32 NewMaxHP)
{
	if (!HB_Segments)
	{
		return;
	}
	
	if (NewMaxHP != CachedMaxHealth)
	{
		RebulidSegment(NewMaxHP);
	}
	
	for (int32 i = 0; i < SegmentImages.Num(); ++i)
	{
		if (UImage* SegmentImage = SegmentImages[i])
		{
			SegmentImage->SetBrushFromTexture(i < NewCurrentHP ? FiledHPTexture : EmptyHPTexture);
		}
	}
}

void UGS_HPCountWidget::RebulidSegment(int32 NewMaxHP)
{
	CachedMaxHealth = NewMaxHP;
	
	HB_Segments->ClearChildren();
	SegmentImages.Empty();

	for (int32 i = 0; i < NewMaxHP; ++i)
	{
		UImage* SegmentImage = NewObject<UImage>(this);
		SegmentImage->SetBrushFromTexture(FiledHPTexture);
		SegmentImage->SetDesiredSizeOverride(SegmentSize);
		
		if (UHorizontalBoxSlot* HB_Slot = HB_Segments->AddChildToHorizontalBox(SegmentImage))
		{
			HB_Slot->SetPadding(FMargin(2.f));
		}
		
		SegmentImages.Add(SegmentImage);
	}
}
