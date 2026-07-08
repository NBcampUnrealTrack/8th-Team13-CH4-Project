#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_HPCountWidget.generated.h"

class UImage;
class UHorizontalBox;

UCLASS()
class GANG_SQUIRREL_API UGS_HPCountWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable,Category="UI")
	void SetHealth(int32 NewCurrentHP, int32 NewMaxHP);
	
protected:
#pragma region BindWidget
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UHorizontalBox> HB_Segments;
#pragma endregion 
	
	UPROPERTY(EditDefaultsOnly,Category="UI")
	TObjectPtr<UTexture2D> FiledHPTexture;
	UPROPERTY(EditDefaultsOnly,Category="UI")
	TObjectPtr<UTexture2D> EmptyHPTexture;
	UPROPERTY(EditDefaultsOnly,Category="UI")
	FVector2D SegmentSize = FVector2D(32.f,32.f);
	
private:
	UPROPERTY()
	TArray<TObjectPtr<UImage>> SegmentImages;
	
	int32 CachedMaxHealth = -1;
	
	void RebulidSegment(int32 NewMaxHP);
	
	
	
};
