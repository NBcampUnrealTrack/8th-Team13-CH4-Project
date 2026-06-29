

#include "GSPlayerNameTag.h"
#include "Components/TextBlock.h"

void UGSPlayerNameTag::SetNickname(const FString& NewName)
{
    if (IsValid(TXT_Nickname))
    {
        TXT_Nickname->SetText(FText::FromString(NewName));
    }
}