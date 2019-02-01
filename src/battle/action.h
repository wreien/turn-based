#ifndef BATTLE_ACTION_H_INCLUDED
#define BATTLE_ACTION_H_INCLUDED

#include <vector>
#include <variant>
#include <string>

#include "skillref.h"

namespace battle {


class Entity;
class PlayerController;


/// Collate different action types
/// TODO: make references pointers (or std::reference_wrapper)?
namespace action {

    /// Enter defensive mode
    /// TODO: wrap in with skills?
    struct Defend {};

    /// Attempt to run away
    struct Flee {};

    /// Cast a skill at a target (this includes basic attacks)
    /// TODO: field-affecting skills (really should be their own thing, eh?)
    struct Skill {
        SkillRef skill;
        const Entity& target;
    };

    /// Delegate control to a player; should be passed to the rendering system
    ///
    /// Conceptually, the controller provides information to the rendering system
    /// on what is available for the player to do, and keeps returning this action
    /// until a choice has been made
    struct UserChoice {
        PlayerController& controller;
    };

}

/// ADT of possible actions
using Action =
    std::variant< action::Defend
                , action::Skill
                , action::Flee
                , action::UserChoice
                >;


}

#endif // BATTLE_ACTION_H_INCLUDED
