#ifndef BATTLE_SKILLHOOKS_H_INCLUDED
#define BATTLE_SKILLHOOKS_H_INCLUDED

#include <string>
#include <cmath>
#include "skill.h"
#include "entity.h"

namespace battle::skill {

template <battle::Pool pool>
struct PoolCost : CostHook {
    PoolCost(int amt)
        : CostHook{ "pool" + std::to_string(static_cast<int>(pool) + 1) }
        , amt{ amt }
    {}

    [[nodiscard]] bool canPay(const Entity& source) const noexcept override {
        return source.get<pool>() >= amt;
    }

    void pay(Entity& source) const noexcept override {
        source.drain<pool>(source, amt);
    }

    int amt;
};


// Effects
// =============================================================

enum class Stats {
    Physical,
    Magical,
};

struct HealEffect : EffectHook {
    HealEffect(int power)
        : EffectHook{ "heal" }
        , power{ power }
    {}

    void apply(Entity& source, Entity& target, double mod) const noexcept override {
        auto source_stats = source.getStats();
        (void)target; // unused

        // TODO: balance :)
        double amt = power * (0.8 * source_stats.m_atk + 1.2 * source_stats.m_def);
        target.restore<Pool::HP>(source, std::ceil(mod * amt * 0.3));
    }

    int power;
};

template <Stats stats>
struct DamageEffect : EffectHook {
    DamageEffect(int power)
        : EffectHook{ std::string("damage") + (stats == Stats::Physical ? "P" : "M") }
        , power{ power }
    {}

    void apply(Entity& source, Entity& target, double mod) const noexcept override {
        auto source_stats = source.getStats();
        auto target_stats = target.getStats();

        static_assert(stats == Stats::Physical || stats == Stats::Magical);

        double amt = power;
        if constexpr (stats == Stats::Physical)
            amt *= 4.0 * source_stats.p_atk - 2.0 * target_stats.p_def;
        else if constexpr (stats == Stats::Magical)
            amt *= 4.0 * source_stats.m_atk - 2.0 * target_stats.m_def;
        target.drain<Pool::HP>(source, mod * amt);
    }

    int power;
};


}

#endif // BATTLE_SKILLHOOKS_H_INCLUDED
