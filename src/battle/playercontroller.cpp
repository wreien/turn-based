#include "playercontroller.h"
#include "entity.h"
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
    return {
        true,
        true,
        entity.getSkills()
    };
}

void PlayerController::choose(const Action& act) {
    choice.emplace(act);
}


}
