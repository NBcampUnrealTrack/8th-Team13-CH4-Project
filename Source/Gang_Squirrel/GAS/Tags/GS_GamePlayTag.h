#pragma once

#include "NativeGameplayTags.h"

namespace DebuffTag
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Slowed)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Stunned)
}

namespace EventTag
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_Enemy_Hit)
}

namespace AbilityTag
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Attack)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Death);
}
