

#include "GSPlayerNameTag.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"

void UGSPlayerNameTag::SetNickname(const FString& NewName)
{
    if (IsValid(TXT_Nickname))
    {
        TXT_Nickname->SetText(FText::FromString(NewName));
    }
}

void UGSPlayerNameTag::SetReadyState(bool bReady)
{
    if (IsValid(Image_ReadyCheck))
    {
        FLinearColor Color = Image_ReadyCheck->GetColorAndOpacity();
        Color.A = bReady ? 1.f : 0.f;
        Image_ReadyCheck->SetColorAndOpacity(Color);
    }
}
