#include "skill.h"
#include "entity.h"
#include "messages.h"
#include "battleview.h"

namespace battle {

Skill::Skill(const std::string& name, int level)
    : details{ name, level }
{
}

bool Skill::isUsableBy(const Entity& source) const noexcept {
    // note: std::nullopt < x for all x
    if (details.getHPCost() > source.get<Pool::HP>())
        return false;
    if (details.getMPCost() > source.get<Pool::MP>())
        return false;
    if (details.getTechCost() > source.get<Pool::Tech>())
        return false;
    // TODO items
    return true;
}

void Skill::use(MessageLogger& logger,
                Entity& source,
                Entity& target,
                const BattleView& view) const
{
    logger.appendMessage(message::SkillUsed{ *this, source, target });
    processCost(logger, source);
    details.perform(logger, source, target, view);
}

void Skill::processCost(MessageLogger& logger, Entity& source) const noexcept {
    // rewrite with expansion statements when C++20 becomes a thing
    if (auto cost = details.getHPCost(); cost)
        source.drain<Pool::HP>(logger, *cost);
    if (auto cost = details.getMPCost(); cost)
        source.drain<Pool::MP>(logger, *cost);
    if (auto cost = details.getTechCost(); cost)
        source.drain<Pool::Tech>(logger, *cost);
    // TODO items
}

}
