#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "GS_PlayerStart.generated.h"

UCLASS()
class GANG_SQUIRREL_API AGS_PlayerStart : public APlayerStart
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere,Category="Spawn")
	int32 SlotIndex = 0;
};
