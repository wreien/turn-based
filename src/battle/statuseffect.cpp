#include "statuseffect.h"


namespace battle {


using S = StatusEffectId;

StatusEffect::StatusEffect(StatusEffectId id)
    : id{ id }
    , num_turns_remaining{ 0 }
    , mods{ }
{
    // TODO: don't hardcode directly here :)
    switch (id) {
    case S::AttackBoost:
        num_turns_remaining = 3;
        mods.emplace_back(StatType::p_atk, 1, StatModType::additive);
        mods.emplace_back(StatType::m_atk, 1, StatModType::additive);
        break;
    case S::DefenseBreak:
        num_turns_remaining = 3;
        mods.emplace_back(StatType::p_def, -1, StatModType::additive);
        mods.emplace_back(StatType::m_def, -1, StatModType::additive);
    }
}

std::string StatusEffect::getName() const noexcept {
    switch (id) {
    case S::AttackBoost:  return "attack boost";
    case S::DefenseBreak: return "defense break";
    }
    return "?";
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

void StatusEffect::endTurn() noexcept {
    if (num_turns_remaining > 0)
        num_turns_remaining--;
}

bool StatusEffect::isActive() const noexcept {
    return num_turns_remaining != 0;
}


}
