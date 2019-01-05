#ifndef BATTLE_ACTION_H_INCLUDED
#define BATTLE_ACTION_H_INCLUDED

#include <variant>
#include <string>

#include "skillref.h"

namespace battle {


class Entity;
class PlayerController;

namespace action {

    // enter defensive mode
    // TODO: wrap in with skills?
    struct Defend {};

    // attempt to run away
    struct Flee {};

    // cast a skill at a target (this includes basic attacks)
    struct TargetedSkill {
        SkillRef skill;
        Entity& target;
    };

    // cast a skill that takes no target
    // e.g. only affects caster, or affects the entire battlefield
    // TODO: should split up into different targeting types?
    struct UntargetedSkill {
        SkillRef skill;
    };

    // delegate control to a player; should be passed to the rendering system
    //
    // conceptually, the controller provides information to the rendering system
    // on what is available for the player to do, and keeps returning this action
    // until a choice has been made
    struct UserChoice {
        PlayerController& controller;
    };

}

// ADT of possible actions
using Action =
    std::variant< action::Defend
                , action::TargetedSkill
                , action::UntargetedSkill
                , action::Flee
                , action::UserChoice
                >;


}

#endif // BATTLE_ACTION_H_INCLUDED
