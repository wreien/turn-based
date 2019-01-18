#include "stats.h"

namespace battle {


Stats calculateModifiedStats(Stats s, std::vector<StatModifier> mods) noexcept {
    static constexpr auto num_stat_types =
        static_cast<unsigned>(StatType::resist);
    std::array<int, num_stat_types> stat_mult_sum = { 0 };
    std::array<int, num_elements> resist_mult_sum = { 0 };

    auto performMod = [&s](StatType stat, auto modFunc) {
        switch (stat) {
            case StatType::hp:     modFunc(s.max_hp);   break;
            case StatType::mp:     modFunc(s.max_mp);   break;
            case StatType::tech:   modFunc(s.max_tech); break;
            case StatType::p_atk:  modFunc(s.p_atk);    break;
            case StatType::p_def:  modFunc(s.p_def);    break;
            case StatType::m_atk:  modFunc(s.m_atk);    break;
            case StatType::m_def:  modFunc(s.m_def);    break;
            case StatType::skill:  modFunc(s.skill);    break;
            case StatType::evade:  modFunc(s.evade);    break;
            case StatType::speed:  modFunc(s.speed);    break;
            case StatType::resist: break;
        }
    };

    for (const auto& m : mods) {
        switch (m.type) {
        case StatModType::additive:
            if (m.stat == StatType::resist)
                s.setResistance(m.resist, s.getResistance(m.resist) + m.modifier);
            else
                performMod(m.stat, [&m](auto& stat){ stat += m.modifier; });
            break;

        case StatModType::multiplicative:
            if (m.stat == StatType::resist)
                resist_mult_sum[static_cast<unsigned>(m.resist)] += m.modifier;
            else
                stat_mult_sum[static_cast<unsigned>(m.stat)] += m.modifier;
        break;
        }
    }

    for (unsigned i = 0; i < num_stat_types; i++) {
        performMod(static_cast<StatType>(i), [&](auto& stat) {
            stat *= stat_mult_sum[i] / 100.0 + 1.0;
        });
    }

    for (unsigned i = 0; i < num_elements; i++) {
        auto e = static_cast<Element>(i);
        auto r = s.getResistance(e);
        s.setResistance(e, r * (resist_mult_sum[i] / 100.0 + 1.0));
    }

    return s;
}


}
