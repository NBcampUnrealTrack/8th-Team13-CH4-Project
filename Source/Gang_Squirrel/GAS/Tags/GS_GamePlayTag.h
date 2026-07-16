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
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_DropKick)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Death)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Sprint)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Roll)

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_SpeedBoost)
}

namespace StateTag
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_State_Dead)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_State_Sprinting)

}

namespace TeamTag
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Team_Player)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Team_Enemy)
}

