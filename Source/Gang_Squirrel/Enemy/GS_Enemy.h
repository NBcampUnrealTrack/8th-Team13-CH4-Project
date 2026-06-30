#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "GS_Enemy.generated.h"

class UGS_PlayerAttributeSet;

//TODO::Attach AIController, BB,BT
UCLASS()
class GANG_SQUIRREL_API AGS_Enemy : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AGS_Enemy();
	
	virtual void Tick(float DeltaTime) override;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly)
	TObjectPtr<UAbilitySystemComponent> EnemyAbilitySystemComp;
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly)
	TObjectPtr<UGS_PlayerAttributeSet> EnemyAttributeSet;
};
