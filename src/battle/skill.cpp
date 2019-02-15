#include "skill.h"
#include "skillhooks.h"
#include "entity.h"
#include "messages.h"
#include "statuseffect.h"
#include "../util.h"
#include <cmath>
#include <numeric>

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
    // TODO: don't hardcode :)
    if (name == "heal") {
        addHook(PoolCost<Pool::MP>{ 1 });
        addHook(HealEffect{ 2 });
    } else if (name == "attack") {
        addHook(DamageEffect<Method::Physical>{ 2 });
    } else if (name == "attack boost") {
        addHook(PoolCost<Pool::Tech>{ 1 });
        addHook(ApplyStatusEffect{ StatusEffectId::AttackBoost });
    } else if (name == "defense break") {
        addHook(PoolCost<Pool::Tech>{ 1 });
        addHook(ApplyStatusEffect{ StatusEffectId::DefenseBreak });
    }
}

bool Skill::isUsableBy(const Entity& source) const noexcept {
    for (const auto& hook : cost_hooks)
        if (!hook->canPay(source))
            return false;
    return true;
}

void Skill::use(MessageLogger& logger,
                Entity& source,
                Entity& target,
                const std::vector<Entity*>& team
                ) const noexcept
{
    logger.appendMessage(message::SkillUsed{ *this, source, target });

    for (const auto& hook : cost_hooks)
        hook->pay(logger, source);

    using S = skill::Spread;
    if (spread == S::Self || spread == S::Single) {
        if (source.isDead())
            return;
        executeSkill(logger, source, target, target);
    } else {
        for (auto&& entity : team) {
            if (source.isDead())
                return;
            executeSkill(logger, source, *entity, target);
        }
    }

    if (source.isDead())
        return;

    for (const auto& hook : post_hooks)
        hook->apply(logger, source);
}

void Skill::executeSkill(MessageLogger& logger,
                         Entity& source,
                         Entity& target,
                         const Entity& orig
                         ) const noexcept
{
    bool hit = true;
    for (const auto& hook : check_hooks)
        if (!hook->didHit(logger, source, target)) hit = false;
    if (hit) {
        double mod = 1;
        for (const auto& hook : mod_hooks) {
            int m = hook->mod(logger, source, target);
            mod *= static_cast<double>(m + 100) / 100.0;
            mod = std::max(mod, 0.0);
        }

        if (spread == skill::Spread::SemiAoE
            && std::addressof(target) != std::addressof(orig))
        {
            mod *= 0.5;
        }

        for (const auto& hook : effect_hooks)
            hook->apply(logger, source, target, mod);
    }
}

Spread Skill::getSpread() const noexcept {
    return spread;
}

Method Skill::getMethod() const noexcept {
    auto method = Method::Neither;
    for (auto&& hook : effect_hooks) {
        auto opt = hook->getMethod();
        method = opt.value_or(method);
    }
    return method;
}

// TODO: if needed, cache results? (this is also goes for getAccuracy)
// I don't expect it to be an issue, though, since it's unlikely for any skill
// to have more than one or two power/accuracy checks
std::optional<int> Skill::getPower() const noexcept {
    bool has_power = false;
    int power = 0;

    for (auto&& hook : effect_hooks) {
        if (auto opt = hook->getPower()) {
            power += *opt;
            has_power = true;
        }
    }

    if (has_power)
        return power;
    return std::nullopt;
}

std::optional<int> Skill::getAccuracy() const noexcept {
    bool has_accuracy = false;
    double accuracy = 1;

    // TODO: fixed point calculations, maybe?
    // (will that even matter)
    for (auto&& hook : check_hooks) {
        if (auto opt = hook->getAccuracy()) {
            accuracy *= static_cast<double>(*opt) / 100.0;
            has_accuracy = true;
        }
    }

    if (has_accuracy)
        return static_cast<int>(std::round(accuracy * 100.0));
    return std::nullopt;
}

Element Skill::getElement() const noexcept {
    auto element = Element::Neutral;
    // OK, this is *slightly* ridiculous ;)
    for (auto&& hook : mod_hooks)
        if (auto var = hook->getDescription(); auto pe = std::get_if<Element>(&var))
            element = *pe;
    return element;
}

std::string Skill::getCostDescription() const noexcept {
    using namespace std::string_literals;

    auto fn = [](std::string acc, const std::unique_ptr<CostHook>& hook) {
        auto m = hook->getMessage();
        auto e = std::empty(acc);
        if (std::empty(m))
            return acc;
        return std::move(acc) + (e ? ""s : " + "s) + std::move(m);
    };

    auto cost = std::accumulate(std::begin(cost_hooks), std::end(cost_hooks), ""s, fn);
    return std::empty(cost) ? "None"s : cost;
}

std::vector<std::string> Skill::getModifierDescriptions() const noexcept {
    std::vector<std::string> descriptions;
    for (auto&& hook : mod_hooks)
        if (auto x = hook->getDescription(); auto ps = std::get_if<std::string>(&x))
            descriptions.push_back(*ps);
    return descriptions;
}

std::vector<std::string> Skill::getUseEffectDescriptions() const noexcept {
    std::vector<std::string> descriptions;
    for (auto&& hook : post_hooks)
        descriptions.push_back(hook->getMessage());
    return descriptions;
}


}
