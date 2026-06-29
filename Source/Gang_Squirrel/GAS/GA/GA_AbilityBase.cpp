#include "GA_AbilityBase.h"

UGA_AbilityBase::UGA_AbilityBase()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}
