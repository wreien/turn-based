#include "battle/skill.h"

#include "battle/battlesystem.h"
#include "battle/entity.h"
#include "battle/messages.h"

namespace battle {

Skill::Skill(const std::string& name, int level)
    : details{ name, level }
{
}

bool Skill::isUsableBy(const Entity& source) const noexcept {
    // note: std::nullopt < x for all x
    if (details.getHealthCost() > source.get<Pool::Health>())
        return false;
    if (details.getManaCost() > source.get<Pool::Mana>())
        return false;
    if (details.getTechCost() > source.get<Pool::Tech>())
        return false;
    // TODO items
    return true;
}

void Skill::use(MessageLogger& logger,
                Entity& source,
                Entity& target,
                BattleSystem& system) const
{
    logger.appendMessage(message::SkillUsed{ *this, source, target });
    processCost(logger, source);
    details.perform(logger, source, target, system);
}

void Skill::processCost(MessageLogger& logger, Entity& source) const noexcept {
    // rewrite with expansion statements when C++20 becomes a thing
    if (auto cost = details.getHealthCost(); cost)
        source.drain<Pool::Health>(logger, *cost);
    if (auto cost = details.getManaCost(); cost)
        source.drain<Pool::Mana>(logger, *cost);
    if (auto cost = details.getTechCost(); cost)
        source.drain<Pool::Tech>(logger, *cost);
    // TODO items
}

}
