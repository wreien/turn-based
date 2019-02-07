#ifndef BATTLE_SKILLHOOKS_H_INCLUDED
#define BATTLE_SKILLHOOKS_H_INCLUDED

#include <string>
#include <cmath>
#include "skill.h"
#include "entity.h"
#include "statuseffect.h"

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

    void pay(MessageLogger& logger, Entity& source) const noexcept override {
        source.drain<pool>(logger, amt);
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

    void apply(MessageLogger& logger,
            Entity& source, Entity& target, double mod) const noexcept override
    {
        auto source_stats = source.getStats();

        // TODO: balance :)
        double raw = power * (0.8 * source_stats.m_atk + 1.2 * source_stats.m_def);
        double heal = std::ceil(mod * raw);
        target.restore<Pool::HP>(logger, static_cast<int>(heal));
    }

    int power;
};

template <Stats stats>
struct DamageEffect : EffectHook {
    DamageEffect(int power)
        : EffectHook{ std::string("damage") + (stats == Stats::Physical ? "P" : "M") }
        , power{ power }
    {}

    void apply(MessageLogger& logger,
            Entity& source, Entity& target, double mod) const noexcept override
    {
        auto source_stats = source.getStats();
        auto target_stats = target.getStats();

        static_assert(stats == Stats::Physical || stats == Stats::Magical);

        int atk = 0;
        int def = 0;
        if constexpr (stats == Stats::Physical) {
            atk = source_stats.p_atk;
            def = target_stats.p_def;
        } else if constexpr (stats == Stats::Magical) {
            atk = source_stats.m_atk;
            def = target_stats.m_def;
        }

        double raw = std::max(4 * atk - 2 * def, 0);
        double dmg = power * raw * mod;
        target.drain<Pool::HP>(logger, static_cast<int>(dmg));
    }

    int power;
};

// TODO: differentiate between effects targeted at enemies and retributive effects
// TODO: more than one status effect? Is that something we want?
struct ApplyStatusEffect : EffectHook {
    template <typename... Args>
    ApplyStatusEffect(Args&&... args)
        : EffectHook{ "status" } // currently only one status effect per skill
        , effect{ std::forward<Args>(args)... }
    {}

    // TODO: some sort of "luck" stat affecting chance of applying status effects?
    // Do status effects get affected by elemental resistances?
    void apply(MessageLogger& logger,
               [[maybe_unused]] Entity& source, Entity& target,
               [[maybe_unused]] double mod)
        const noexcept override
    {
        target.applyStatusEffect(logger, effect);
    }

    StatusEffect effect;
};


}

#endif // BATTLE_SKILLHOOKS_H_INCLUDED
