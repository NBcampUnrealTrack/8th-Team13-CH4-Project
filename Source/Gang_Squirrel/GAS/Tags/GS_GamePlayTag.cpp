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
	UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_DropKick,"Ability.DropKick")
	UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_Death,"Ability.Death")
	UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_Sprint, "Ability.Sprint")
	UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_Roll, "Ability.Roll")

}

namespace StateTag
{
	UE_DEFINE_GAMEPLAY_TAG(TAG_State_Dead,"State.Dead")
	UE_DEFINE_GAMEPLAY_TAG(TAG_State_Sprinting, "State.Sprinting")

}

namespace TeamTag
{
	UE_DEFINE_GAMEPLAY_TAG(TAG_Team_Player,"Team.Player")
	UE_DEFINE_GAMEPLAY_TAG(TAG_Team_Enemy,"Team.Enemy")
}

