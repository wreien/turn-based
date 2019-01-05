#ifndef BATTLE_SKILL_H_INCLUDED
#define BATTLE_SKILL_H_INCLUDED

#include <string>
#include <string_view>
#include <optional>
#include "stats.h"

namespace battle {


/// Determines the stats that a skill uses in calculations
enum class AttackMethod {
    Physical,  ///< damage based on `p_atk' and `p_def'
    Magical,   ///< damage based on `m_atk' and `m_def'
};

/// Determines the spread of an attack
enum class AttackSpread {
    Self,    ///< targets source
    Single,  ///< targets one entity
    SemiAoE, ///< targets one entity, with spread to rest of their team
    AoE,     ///< targets a whole team
    Field,   ///< untargeted
};


/// Encapsulates a skill
struct Skill {
    /// Create the a specified skill
    explicit Skill(const std::string& name);

    std::string name;    ///< the name of the skill
    AttackMethod method; ///< the skill's damaging method
    AttackSpread spread; ///< the AoE-ness of the skill

    int power;           ///< the skill's base power (damage)
    double hit_chance;   ///< the base chance of scoring a hit
};


/// get the percentage chance of landing a hit
[[nodiscard]] constexpr double
chanceToHit(const Skill& skill, const Stats& source, const Stats& target) noexcept {
    return skill.hit_chance + source.skill - target.evade;
}

/// get the damage done by a skill
[[nodiscard]] constexpr int
damageDealt(const Skill& skill, const Stats& source, const Stats& target) noexcept {
    int atk = 0, def = 0;
    switch (skill.method) {
    case AttackMethod::Physical:
        atk = source.p_atk;
        def = target.p_def;
        break;
    case AttackMethod::Magical:
        atk = source.m_atk;
        def = target.m_def;
        break;
    }
    return skill.power * (4 * atk - 2 * def);
}


}


#endif // BATTLE_SKILL_H_INCLUDED
