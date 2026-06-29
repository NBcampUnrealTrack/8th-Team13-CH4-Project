#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GS_Enemy.generated.h"

UCLASS()
class GANG_SQUIRREL_API AGS_Enemy : public ACharacter
{
	GENERATED_BODY()

public:
	
	AGS_Enemy();

protected:
	
	virtual void BeginPlay() override;

public:
	
	virtual void Tick(float DeltaTime) override;

	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
