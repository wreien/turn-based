#include "skill.h"
#include "skillhooks.h"
#include "entity.h"
#include <cmath>

namespace battle {

using namespace skill;

Skill::Skill(const std::string& name)
    : name{ name }
    , spread{ Spread::Single }
    , power_sources{ }
    , accuracy_sources{ }
    , cost_hooks{ }
    , check_hooks{ }
    , mod_hooks{ }
    , effect_hooks{ }
    , post_hooks{ }
{
    // TODO: don't hardcode :)
    if (name == "heal") {
        addHook(PoolCost<Pool::MP>{ 1 });
        addHook(HealEffect{ 2 });
    } else if (name == "attack") {
        addHook(DamageEffect<skill::Stats::Physical>{ 2 });
    }
}

bool Skill::isUsableBy(const Entity& source) const noexcept {
    for (const auto& hook : cost_hooks)
        if (!hook->canPay(source))
            return false;
    return true;
}

void Skill::use(Entity& source,
                Entity& target,
                const std::vector<Entity*>& team
                ) const noexcept
{
    for (const auto& hook : cost_hooks)
        hook->pay(source);

    using S = skill::Spread;
    if (spread == S::Self || spread == S::Single) {
        if (source.isDead())
            return;
        executeSkill(source, target, target);
    } else {
        for (auto&& entity : team) {
            if (source.isDead())
                return;
            executeSkill(source, *entity, target);
        }
    }

    if (source.isDead())
        return;

    for (const auto& hook : post_hooks)
        hook->apply(source);
}

void Skill::executeSkill(Entity& source,
                         Entity& target,
                         const Entity& orig
                         ) const noexcept
{
    bool hit = true;
    for (const auto& hook : check_hooks)
        if (!hook->didHit(source, target)) hit = false;
    if (hit) {
        double mod = 1;
        for (const auto& hook : mod_hooks) {
            int m = hook->mod(source, target);
            mod *= static_cast<double>(m + 100) / 100.0;
            mod = std::max(mod, 0.0);
        }

        if (spread == skill::Spread::SemiAoE
            && std::addressof(target) != std::addressof(orig))
        {
            mod *= 0.5;
        }

        for (const auto& hook : effect_hooks)
            hook->apply(source, target, mod);
    }
}

Spread Skill::getSpread() const noexcept {
    return spread;
}

// TODO: if needed, cache results? (this is also goes for getAccuracy)
// I don't expect it to be an issue, though, since it's unlikely for any skill
// to have more than one or two power/accuracy checks
std::optional<int> Skill::getPower() const noexcept {
    if (std::empty(power_sources))
        return std::nullopt;
    int power = 0;
    for (auto&& entry : power_sources)
        power += entry.second;
    return power;
}

std::optional<int> Skill::getAccuracy() const noexcept {
    if (std::empty(accuracy_sources))
        return std::nullopt;
    double accuracy = 1;
    for (auto&& entry : accuracy_sources)
        accuracy *= static_cast<double>(entry.second) / 100.0;
    return static_cast<int>(std::round(accuracy * 100.0));
}


}
