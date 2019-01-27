#include "skill.h"
#include "entity.h"

namespace battle {

using namespace skill;

Skill::Skill(const std::string& name)
    : name{ name }
    , spread{ Spread::Single }
    , cost_hooks{ }
    , check_hooks{ }
    , mod_hooks{ }
    , effect_hooks{ }
    , post_hooks{ }
{
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

std::optional<int> Skill::getPower() const noexcept {
    // TODO
    return std::nullopt;
}

std::optional<int> Skill::getAccuracy() const noexcept {
    // TODO
    return std::nullopt;
}


}
