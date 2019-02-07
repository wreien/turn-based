#include "playercontroller.h"
#include "entity.h"
#include "../util.h"
#include <algorithm>

namespace battle {


PlayerController::PlayerController(Entity& entity)
    : entity{ entity }
    , choice{ std::nullopt }
{
}

Action PlayerController::go(const BattleView&) {
    auto c = choice.value_or(action::UserChoice{ *this });
    choice = std::nullopt;
    return c;
}

UserOptions PlayerController::options() const {
    auto skills = entity.getSkills();
    skills.erase(std::remove_if(
        std::begin(skills), std::end(skills),
        [&](auto&& s){ return !s->isUsableBy(entity); }
    ), std::end(skills));
    return {
        true,
        true,
        skills
    };
}

void PlayerController::choose(const Action& act) {
    choice.emplace(act);
}


}
