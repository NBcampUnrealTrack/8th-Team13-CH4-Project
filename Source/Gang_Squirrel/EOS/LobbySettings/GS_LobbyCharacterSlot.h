#pragma once

#include "CoreMinimal.h"
#include "Engine/TargetPoint.h"
#include "GS_LobbyCharacterSlot.generated.h"

UCLASS()
class GANG_SQUIRREL_API AGS_LobbyCharacterSlot : public ATargetPoint
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere,Category="Lobby")
	int32 SlotIndex = 0;
};
