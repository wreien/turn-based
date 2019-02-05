#include "statuseffect.h"

namespace battle {


StatusEffect::StatusEffect(std::string name_)
    : name{ std::move(name_) }
    , num_turns_remaining{ 0 }
    , mods{ }
{
    // TODO: don't hardcode directly here :)
    if (name == "attack boost") {
        num_turns_remaining = 3;
        mods.emplace_back(StatType::p_atk, 2, StatModType::additive);
        mods.emplace_back(StatType::m_atk, 2, StatModType::additive);
    } else if (name == "defense break") {
        num_turns_remaining = 3;
        mods.emplace_back(StatType::p_def, -2, StatModType::additive);
        mods.emplace_back(StatType::m_def, -2, StatModType::additive);
    }
}

EffectDuration StatusEffect::getEffectDuration() const noexcept {
    if (num_turns_remaining >= 0)
        return EffectDuration::Temporary;
    if (num_turns_remaining == -1)
        return EffectDuration::Battle;
    return EffectDuration::Permanent;
}

std::optional<int> StatusEffect::getRemainingTurns() const noexcept {
    if (getEffectDuration() != EffectDuration::Temporary)
        return std::nullopt;
    return num_turns_remaining;
}

bool StatusEffect::endTurn() noexcept {
    if (num_turns_remaining > 0)
        num_turns_remaining--;
    return num_turns_remaining == 0;
}


}
