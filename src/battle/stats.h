#ifndef BATTLE_STATS_H_INCLUDED
#define BATTLE_STATS_H_INCLUDED

#include <array>
#include "element.h"

namespace battle {


/// Defines a stat block for an entity
struct Stats {
    // pools for health, magic, and skills respectively
    int max_hp;
    int max_mp;
    int max_tech;

    // modifiers for physical and magical damage
    int p_atk;
    int p_def;

    int m_atk;
    int m_def;

    // hit chance (as a percentage)
    int skill;
    int evade;

    // turn order
    int speed;

    // base resistances
    [[nodiscard]] constexpr int getResistance(Element e) const noexcept
        { return resist[static_cast<decltype(resist)::size_type>(e)]; }
    constexpr void setResistance(Element e, int value) noexcept
        { resist[static_cast<decltype(resist)::size_type>(e)] = value; }
    std::array<int, num_elements> resist;
};

/// Lists all the different options for a stat modifier
enum class StatType {
    hp,     ///< max health
    mp,     ///< max mana
    tech,   ///< max tech
    p_atk,  ///< physical attack
    p_def,  ///< physical defense
    m_atk,  ///< magical attack
    m_def,  ///< magical defense
    skill,  ///< hit chance (percentage)
    evade,  ///< evade chance (percentage)
    speed,  ///< move speed/turn order
    resist  ///< some elemental resistance
};

/// Ways a modifier affects a statistic
enum class StatModType {
    additive,       ///< Add the raw values together (comes before multiplicative)
    multiplicative, ///< Apply the sum of all multiplicatives as a percentage mod
};

/// Modifies a stat in some way
/// TODO: everything `int`? `double`? `union`? template?
struct StatModifier {
    StatType stat;     ///< the stat to modify
    Element resist;    ///< the resistance to modify (applicable iff stat == resist)
    int modifier;      ///< the modification
    StatModType type;  ///< how to calculate the modification
};


}

#endif // BATTLE_STATS_H_INCLUDED