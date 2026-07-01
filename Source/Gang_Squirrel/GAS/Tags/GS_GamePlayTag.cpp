#include "GS_GamePlayTag.h"

namespace DebuffTag
{
	UE_DEFINE_GAMEPLAY_TAG(TAG_Status_Slowed, "Status.Slowed")
	UE_DEFINE_GAMEPLAY_TAG(TAG_Status_Stunned, "Status.Stunned")
}

namespace EventTag
{
	UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Enemy_Hit, "Event.Enemy.Hit")
}

namespace AbilityTag
{
	UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_Attack,"Ability.Attack")
}