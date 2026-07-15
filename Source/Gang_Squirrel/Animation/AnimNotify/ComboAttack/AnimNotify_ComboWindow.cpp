#include "AnimNotify_ComboWindow.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Gang_Squirrel/GAS/GA/Attack/IGA_AttackTraceInterface.h"
#include "Gang_Squirrel/GAS/Tags/GS_GamePlayTag.h"

void UAnimNotify_ComboWindow::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                     const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	
	AActor* OwnerActor = MeshComp ? MeshComp->GetOwner() : nullptr;
	UAbilitySystemComponent* ASC = OwnerActor ? UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerActor) : nullptr;
	if (!ASC)
	{
		return;
	}
	UGameplayAbility* ActiveAttackAbility = nullptr;
	for (FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
	{
		if (Spec.IsActive() && Spec.Ability && Spec.Ability->AbilityTags.HasTag(AbilityTag::TAG_Ability_Attack))
		{
			ActiveAttackAbility = Spec.GetPrimaryInstance();
			break;
		}
	}
	
	if (IGA_AttackTraceInterface* ComboInstigator = Cast<IGA_AttackTraceInterface>(ActiveAttackAbility))
	{
		ComboInstigator->OnComboWindowOpen();
	}
	
}
