#include "npccontroller.h"
#include "battleview.h"

namespace battle {

NPCController::NPCController(Entity& entity)
    : entity{ entity }
{
}

Action NPCController::go(const BattleView&) {
    // TODO: actually do something useful here
    return action::Defend{};
}

}
