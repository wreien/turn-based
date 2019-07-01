#ifndef BATTLE_STATS_H_INCLUDED
#define BATTLE_STATS_H_INCLUDED

#include <array>
#include <vector>
#include "battle/element.h"

namespace battle {


/// Defines a stat block for an entity
struct Stats {
    // pools for health, magic, and skills respectively
    int max_health;
    int max_mana;
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
    int react;

    // base resistances
    [[nodiscard]] constexpr int getResistance(Element e) const noexcept
        { return resist[static_cast<decltype(resist)::size_type>(e)]; }
    constexpr void setResistance(Element e, int value) noexcept
        { resist[static_cast<decltype(resist)::size_type>(e)] = value; }
    std::array<int, num_elements> resist = {};
};

/// Lists valid "pooled" stats
enum class Pool {
    Health,  ///< health pool
    Mana,    ///< mana pool
    Tech,    ///< tech pool
};

inline std::string to_string(Pool pool) noexcept {
    switch (pool) {
    case Pool::Health: return "HP";
    case Pool::Mana: return "MP";
    case Pool::Tech: return "TP";
    }
    return "??";
}

/// Lists all the different options for a stat modifier
enum class StatType {
    health, ///< max health
    mana,   ///< max mana
    tech,   ///< max tech
    p_atk,  ///< physical attack
    p_def,  ///< physical defense
    m_atk,  ///< magical attack
    m_def,  ///< magical defense
    skill,  ///< hit chance (percentage)
    evade,  ///< evade chance (percentage)
    react,  ///< move speed/turn order
    resist, ///< some elemental resistance
};

/// Ways a modifier affects a statistic
enum class StatModType {
    additive,       ///< Add the raw values together (comes before multiplicative)
    multiplicative, ///< Apply the sum of all multiplicatives as a percentage mod
};

/// Modifies a stat in some way
/// TODO: everything `int`? `double`? `union`? template?
struct StatModifier {
    StatModifier(StatType stat, int modifier, StatModType type)
        : stat{ stat }, resist{ Element::Neutral }, modifier{ modifier }, type{ type }
    {}

    StatModifier(Element resist, int modifier, StatModType type)
        : stat{ StatType::resist }, resist{ resist }, modifier{ modifier }, type{ type }
    {}

    StatType stat;     ///< the stat to modify
    Element resist;    ///< the resistance to modify (applicable iff stat == resist)
    int modifier;      ///< the modification
    StatModType type;  ///< how to calculate the modification
};

/// Apply a collection of stat modifiers to a stat block
[[nodiscard]] Stats
calculateModifiedStats(Stats s, const std::vector<StatModifier>& mods) noexcept;

}

#endif // BATTLE_STATS_H_INCLUDED
